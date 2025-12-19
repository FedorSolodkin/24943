#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char *argv[]){
    int status;
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program> [args...]\n", argv[0]);
        return 1;
    }
    if (fork() == 0) {
        execvp(argv[1], &argv[1]);
        perror(argv[1]);
        exit(127);
    }
    wait(&status);
    if (WIFEXITED(status))
        printf("exit status: %d\n", WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("killed by signal: %d\n", WTERMSIG(status));
    else
        printf("child status: 0x%x\n", status);
    return 0;
}
