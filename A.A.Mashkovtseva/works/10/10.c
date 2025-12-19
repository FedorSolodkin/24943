#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    int status;

    if (argc == 1) {
        printf("You should write a command (with arguments if needed)\n");
        return 1;
    }

    if (fork() == 0) {
        execvp(argv[1], &argv[1]);
        perror(argv[1]);
        return 2;
    }

    wait(&status);
    printf("finishing status: %d\n", WEXITSTATUS(status));
    return 0;
}