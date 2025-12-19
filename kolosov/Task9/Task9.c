#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
    pid_t pid;
    int status;

    pid = fork();

    if (pid == 0) {
        execlp("cat", "cat", "text.txt", (char *)0);
    } else {
        printf("Parent: child working \n");
        waitpid(pid, &status, 0);
        printf("Parent: child finish \n");
    }

    return 0;
}
