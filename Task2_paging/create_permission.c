#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

void showPermissions(const char *filename)
{
    struct stat fileInfo;

    if (stat(filename, &fileInfo) == -1)
    {
        perror("stat");
        return;
    }

    char permission[11];

    permission[0] = '-';
    permission[1] = (fileInfo.st_mode & S_IRUSR) ? 'r' : '-';
    permission[2] = (fileInfo.st_mode & S_IWUSR) ? 'w' : '-';
    permission[3] = (fileInfo.st_mode & S_IXUSR) ? 'x' : '-';
    permission[4] = (fileInfo.st_mode & S_IRGRP) ? 'r' : '-';
    permission[5] = (fileInfo.st_mode & S_IWGRP) ? 'w' : '-';
    permission[6] = (fileInfo.st_mode & S_IXGRP) ? 'x' : '-';
    permission[7] = (fileInfo.st_mode & S_IROTH) ? 'r' : '-';
    permission[8] = (fileInfo.st_mode & S_IWOTH) ? 'w' : '-';
    permission[9] = (fileInfo.st_mode & S_IXOTH) ? 'x' : '-';
    permission[10] = '\0';

    printf("File : %s\n", filename);
    printf("Permission (Octal): %o\n", fileInfo.st_mode & 0777);
    printf("Permission (rwx)  : %s\n", permission);
    printf("Size              : %ld bytes\n\n", (long)fileInfo.st_size);
}

int main()
{
    const char *filename = "secure_document.txt";

    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0600);

    if (fd == -1)
    {
        perror("open");
        return 1;
    }

    char text[] = "This file is created to demonstrate Linux file permissions.\n";

    if (write(fd, text, strlen(text)) == -1)
    {
        perror("write");
        close(fd);
        return 1;
    }

    close(fd);

    printf("Step 1: File created with permission 0600\n");
    showPermissions(filename);

    chmod(filename, 0644);
    printf("Step 2: Permission changed to 0644\n");
    showPermissions(filename);

    chmod(filename, 0640);
    printf("Step 3: Permission changed to 0640\n");
    showPermissions(filename);

    chmod(filename, 0750);
    printf("Step 4: Permission changed to 0750\n");
    showPermissions(filename);

    printf("Program completed successfully.\n");

    return 0;
}