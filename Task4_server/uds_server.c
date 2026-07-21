#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "/tmp/os_coursework.sock"

int main(void)
{
    // Remove old socket file if it exists
    unlink(SOCK_PATH);

    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_un addr;

    memset(&addr, 0, sizeof(addr));

    addr.sun_family = AF_UNIX;

    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(server_fd);
        return EXIT_FAILURE;
    }

    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        close(server_fd);
        unlink(SOCK_PATH);
        return EXIT_FAILURE;
    }

    printf("[UDS Server] Listening on %s\n", SOCK_PATH);

    // Accept two client connections
    for (int i = 0; i < 2; i++)
    {
        int client_fd = accept(server_fd, NULL, NULL);

        if (client_fd < 0)
        {
            perror("accept");
            continue;
        }

        char buf[256];

        ssize_t n = read(client_fd, buf, sizeof(buf) - 1);

        if (n > 0)
        {
            buf[n] = '\0';

            printf("[UDS Server] Received: %s\n", buf);

            char reply[300];

            int len = snprintf(reply,
                               sizeof(reply),
                               "ACK: %s",
                               buf);

            write(client_fd, reply, len);
        }

        close(client_fd);
    }

    close(server_fd);

    unlink(SOCK_PATH);

    return 0;
}