#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int fd[2], pid;

    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }

    pid = fork();
    if (pid > 0) {
        close(fd[0]);
        const char *msg = "hello WoRld!";
        write(fd[1], msg, strlen(msg));
        close(fd[1]);
    }
    else if (pid == 0) {
        close(fd[1]);
        char c;
        while (read(fd[0], &c, 1) > 0) {
            c = toupper((unsigned char)c);
            write(STDOUT_FILENO, &c, 1);
        }
        write(STDOUT_FILENO, "\n", 1);
        close(fd[0]);
    }
    else {
        perror("fork");
        return 1;
    }

    return 0;
}