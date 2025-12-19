#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    pid_t pid;
    int status;

    if (argc < 2) {
        fprintf(stderr, "%s : no args...\n", argv[0]);
        return 1;
    }

    pid = fork();

    if (pid == 0) {
        execvp(argv[1], &argv[1]);
        perror("execvp");
        return 1;
    } else {
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Exit code: %d\n", WEXITSTATUS(status));
        } else {
            printf("error\n");
        }
    }

    return 0;
}
