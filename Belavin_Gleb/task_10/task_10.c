#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    pid_t pid;
    int status;
    int i;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        fprintf(stderr, "Example: %s ls -l -a\n", argv[0]);
        exit(1);
    }

    printf("Executing: ");
    for (i = 1; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");

    pid = fork();

    if (pid == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) {
        execvp(argv[1], &argv[1]);
        perror("execvp failed");
        exit(127);
    } else {
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Command completed with exit code: %d\n", exit_code);
            return exit_code;
        } else if (WIFSIGNALED(status)) {
            int signal_num = WTERMSIG(status);
            printf("Command terminated by signal: %d\n", signal_num);
            return 128 + signal_num; 
        } else {
            printf("Command terminated dont work correct\n");
            return 1;
        }
    }
}
