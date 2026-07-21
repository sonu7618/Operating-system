#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    int fd[2];

    if (pipe(fd) == -1)
    {
        perror("pipe");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return EXIT_FAILURE;
    }

    // Child process
    if (pid == 0)
    {
        close(fd[0]);

        const char *messages[] =
        {
            "Hello from the child process!",
            "This message travels through an anonymous pipe.",
            "Pipes only work between parent and child processes."
        };

        for (int i = 0; i < 3; i++)
        {
            write(fd[1], messages[i], strlen(messages[i]) + 1);
            usleep(200000);
        }

        close(fd[1]);

        _exit(0);
    }

    // Parent process
    else
    {
        close(fd[1]);

        char buffer[256];

        printf("[Parent] Waiting for messages from child (PID = %d)\n", pid);

        ssize_t n;

        while ((n = read(fd[0], buffer, sizeof(buffer))) > 0)
        {
            printf("[Parent] Received: %s\n", buffer);
        }

        close(fd[0]);

        int status;

        waitpid(pid, &status, 0);

        printf("[Parent] Child exited with status %d\n",
               WEXITSTATUS(status));
    }

    return 0;
}