#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s \"text\"\n", argv[0]);
        return 1;
    }

    int pipefd[2];
    pipe(pipefd);

    if (fork() == 0) {
        close(pipefd[1]);
        char c;
        while (read(pipefd[0], &c, 1) > 0) {
            putchar(toupper((unsigned char)c));
        }
        putchar('\n');
        close(pipefd[0]);
    } else {
        close(pipefd[0]);
        write(pipefd[1], argv[1], strlen(argv[1]));
        close(pipefd[1]);
    }
    return 0;
}
