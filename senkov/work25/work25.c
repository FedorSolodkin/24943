#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {
    int fildes[2];
    if (pipe(fildes) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        close(fildes[1]); // Закрываем запись в потомке

        char c;
        ssize_t n;
        while ((n = read(fildes[0], &c, 1)) > 0) {
            printf("%c", toupper((unsigned char)c));
        }
        if (n == -1) {
            perror("read");
        }

        close(fildes[0]);
        exit(0); // Явное завершение потомка
    } else {
        close(fildes[0]); // Закрываем чтение в родителе

        char *text = "Hello, World!\0";
        for (char *ptr = text; *ptr != 0; ptr++) {
            write(fildes[1], ptr, 1);
        }

        close(fildes[1]);
        wait(NULL);
    }

    return 0;
}
