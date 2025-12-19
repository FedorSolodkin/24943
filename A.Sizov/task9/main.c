#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        execl("/usr/bin/cat", "cat", "/path/to/largefile.txt", (char *)NULL);
        perror("execl failed");
        exit(EXIT_FAILURE);
    } else {
        printf("...\n");
    }

    return 0;
}
