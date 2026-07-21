#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "/tmp/os_coursework.sock"

int main(int argc, char *argv[])
{
    // Use the command-line message or a default message
    const char *message = (argc > 1)
                            ? argv[1]
                            : "Hello over a Unix domain socket";

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (fd < 0)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_un addr;

    memset(&addr, 0, sizeof(addr));

    addr.sun_family = AF_UNIX;

    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        close(fd);
        return EXIT_FAILURE;
    }

    write(fd, message, strlen(message));

    printf("[UDS Client] Sent: %s\n", message);

    char buf[300];

    ssize_t n = read(fd, buf, sizeof(buf) - 1);

    if (n > 0)
    {
        buf[n] = '\0';
        printf("[UDS Client] Received: %s\n", buf);
    }

    close(fd);

    return 0;
}