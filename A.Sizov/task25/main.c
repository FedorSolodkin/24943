#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main() {
    int pipefd[2];
    pid_t pid;
    char buf[BUFFER_SIZE];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }

    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        // ===== Дочерний процесс =====
        close(pipefd[1]);

        ssize_t n;
        while ((n = read(pipefd[0], buf, BUFFER_SIZE)) > 0) {
            for (ssize_t i = 0; i < n; i++) {
                buf[i] = toupper((unsigned char)buf[i]);
            }
            write(STDOUT_FILENO, buf, n);
        }

        close(pipefd[0]);
        exit(0);

    } else {
        // ===== Родительский процесс =====
        close(pipefd[0]);

        printf("Введите текст (Ctrl+D для окончания):\n");

        while (fgets(buf, BUFFER_SIZE, stdin) != NULL) {
            write(pipefd[1], buf, strlen(buf));
        }

        close(pipefd[1]);
        wait(NULL);
    }

    return 0;
}
