#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

int main(){
    pid_t pid, ret;
    int status;

    if ((pid = fork()) == 0){
        execl("/bin/cat", "cat", "/etc/passwd", (char *)0);
        _exit(127);
    } else {
        printf("parent: waiting for child: %ld\n", (long)pid);
        ret = wait(&status);
        printf("parent: wait's return value: %ld,", (long)ret);
        printf("child's status: %d\n", WEXITSTATUS(status));
        exit(0);
    }
}
