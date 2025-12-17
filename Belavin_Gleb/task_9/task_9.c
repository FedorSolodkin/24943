#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid;
    int status;

    pid = fork();

    if (pid == -1) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) {
        execlp("cat", "cat", "/etc/services", NULL);
        perror("execlp failed");
        exit(1);
    } else {
        printf("Parent: Child process created with PID %d\n", pid);
        printf("Parent: First parent message\n");
        printf("Parent: Second parent message\n");
        printf("Parent: Third parent message\n");

        wait(&status);
        
        printf("Parent: Final message - Child process completed\n");
    }

    return 0;
}
