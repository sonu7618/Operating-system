#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 256

// Send a message to the server and receive the reply
int send_and_receive(int fd, const char *msg, char *reply, size_t reply_size)
{
    if (send(fd, msg, strlen(msg), 0) < 0)
    {
        perror("send");
        return -1;
    }

    ssize_t n = recv(fd, reply, reply_size - 1, 0);

    if (n <= 0)
    {
        fprintf(stderr, "Connection closed by server.\n");
        return -1;
    }

    reply[n] = '\0';

    return 0;
}

int main(int argc, char *argv[])
{
    const char *host = (argc > 1) ? argv[1] : "127.0.0.1";
    int port = (argc > 2) ? atoi(argv[2]) : 9090;
    const char *username = (argc > 3) ? argv[3] : "alice";
    const char *password = (argc > 4) ? argv[4] : "P@ssw0rd123";

    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0)
    {
        fprintf(stderr, "Invalid server address: %s\n", host);
        close(fd);
        return EXIT_FAILURE;
    }

    if (connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("[Client] Connected to %s:%d\n", host, port);

    char msg[BUF_SIZE];
    char reply[BUF_SIZE];

    // Authenticate
    snprintf(msg, sizeof(msg), "AUTH %s %s\n", username, password);

    if (send_and_receive(fd, msg, reply, sizeof(reply)) == 0)
    {
        printf("[Client] AUTH -> %s", reply);
    }

    // First addition
    snprintf(msg, sizeof(msg), "ADD 15 27\n");

    if (send_and_receive(fd, msg, reply, sizeof(reply)) == 0)
    {
        printf("[Client] ADD 15 27 -> %s", reply);
    }

    // Second addition
    snprintf(msg, sizeof(msg), "ADD 3.5 6.25\n");

    if (send_and_receive(fd, msg, reply, sizeof(reply)) == 0)
    {
        printf("[Client] ADD 3.5 6.25 -> %s", reply);
    }

    // Disconnect
    snprintf(msg, sizeof(msg), "QUIT\n");

    if (send_and_receive(fd, msg, reply, sizeof(reply)) == 0)
    {
        printf("[Client] QUIT -> %s", reply);
    }

    close(fd);

    printf("[Client] Connection closed.\n");

    return 0;
}