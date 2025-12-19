#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>

int main(int argc, char ** argv){
    if (argc == 1) {
    }

    pid_t pid = fork();

    if (pid == -1){
        printf("error");
    }
    

    if (pid == 0){
        execvp(argv[1], &argv[1]);
    }else{
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("\nChild process (pid: %d) finished with exit code %d\n", pid, WEXITSTATUS(status));
        }else if (WIFSIGNALED(status)) {
            printf("Terminated by signal: %d\n", WTERMSIG(status));
        }
    }

    return 0;
}

