#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdarg.h>

#define BUF_SIZE 256
#define VALID_USER "alice"
#define VALID_PASS "P@ssw0rd123"

typedef struct
{
    int client_fd;
    struct sockaddr_in addr;
} ClientCtx;

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

// Thread-safe logging
void server_log(const char *fmt, ...)
{
    pthread_mutex_lock(&log_mutex);

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    fflush(stdout);

    pthread_mutex_unlock(&log_mutex);
}

// Handle one client connection
void *handle_client(void *arg)
{
    ClientCtx *ctx = (ClientCtx *)arg;
    int fd = ctx->client_fd;

    char client_ip[INET_ADDRSTRLEN];

    inet_ntop(AF_INET,
              &ctx->addr.sin_addr,
              client_ip,
              sizeof(client_ip));

    int authenticated = 0;

    char buf[BUF_SIZE];

    server_log("[Server] Client connected from %s:%d\n",
               client_ip,
               ntohs(ctx->addr.sin_port));

    while (1)
    {
        ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);

        if (n <= 0)
        {
            server_log("[Server] Client %s disconnected\n", client_ip);
            break;
        }

        buf[n] = '\0';
        buf[strcspn(buf, "\r\n")] = '\0';

        char cmd[32] = {0};
        char a1[64] = {0};
        char a2[64] = {0};

        int parsed = sscanf(buf, "%31s %63s %63s", cmd, a1, a2);

        // AUTH command
        if (parsed == 3 && strcmp(cmd, "AUTH") == 0)
        {
            if (strcmp(a1, VALID_USER) == 0 &&
                strcmp(a2, VALID_PASS) == 0)
            {
                authenticated = 1;

                send(fd, "OK\n", 3, 0);

                server_log("[Server] %s authenticated successfully\n",
                           client_ip);
            }
            else
            {
                send(fd, "DENY\n", 5, 0);

                server_log("[Server] %s failed authentication\n",
                           client_ip);
            }
        }

        // ADD command
        else if (parsed == 3 && strcmp(cmd, "ADD") == 0)
        {
            if (!authenticated)
            {
                send(fd, "DENY not_authenticated\n", 23, 0);
            }
            else
            {
                double x = atof(a1);
                double y = atof(a2);

                char reply[BUF_SIZE];

                int len = snprintf(reply,
                                   sizeof(reply),
                                   "RESULT %.2f\n",
                                   x + y);

                send(fd, reply, len, 0);

                server_log("[Server] %s requested ADD %.2f + %.2f = %.2f\n",
                           client_ip,
                           x,
                           y,
                           x + y);
            }
        }

        // QUIT command
        else if (parsed >= 1 && strcmp(cmd, "QUIT") == 0)
        {
            send(fd, "BYE\n", 4, 0);
            break;
        }

        // Invalid command
        else
        {
            send(fd, "ERROR malformed_request\n", 24, 0);
        }
    }

    close(fd);

    free(ctx);

    return NULL;
}

int main(int argc, char *argv[])
{
    int port;

    if (argc > 1)
        port = atoi(argv[1]);
    else
        port = 9090;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    int opt = 1;

    setsockopt(server_fd,
               SOL_SOCKET,
               SO_REUSEADDR,
               &opt,
               sizeof(opt));

    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd,
             (struct sockaddr *)&addr,
             sizeof(addr)) < 0)
    {
        perror("bind");
        close(server_fd);
        return EXIT_FAILURE;
    }

    if (listen(server_fd, 10) < 0)
    {
        perror("listen");
        close(server_fd);
        return EXIT_FAILURE;
    }

    printf("[Server] Listening on port %d...\n", port);

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd,
                               (struct sockaddr *)&client_addr,
                               &client_len);

        if (client_fd < 0)
        {
            perror("accept");
            continue;
        }

        ClientCtx *ctx = malloc(sizeof(ClientCtx));

        if (ctx == NULL)
        {
            perror("malloc");
            close(client_fd);
            continue;
        }

        ctx->client_fd = client_fd;
        ctx->addr = client_addr;

        pthread_t tid;

        if (pthread_create(&tid, NULL, handle_client, ctx) != 0)
        {
            perror("pthread_create");
            close(client_fd);
            free(ctx);
            continue;
        }

        pthread_detach(tid);
    }

    close(server_fd);

    return 0;
}