#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        execlp("cat", "cat", "file.txt", NULL);
    } else {
        wait(NULL);
        printf("Parent process finished.\n");
    }
    return 0;
}
