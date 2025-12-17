#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>

int main(){
    pid_t pid = fork();

    if (pid == -1){
        printf("error");
    }

    if (pid == 0){
        execlp("cat", "cat", "file.txt", NULL);
    } else{
        printf("Parent process is working\n");

        waitpid(pid, NULL, 0);

        printf("Child process (pid: %d) finished\n", pid);
    }
    return 0;
}
