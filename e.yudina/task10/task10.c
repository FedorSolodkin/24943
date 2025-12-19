#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s command [args...]\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        execvp(argv[1], &argv[1]);
        perror("execvp failed");
        return 127;
    }

    int status;
    wait(&status);
    printf("Exit code: %d\n", WEXITSTATUS(status));
    return 0;
}
