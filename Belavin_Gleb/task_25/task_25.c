#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    char c;
    
    pipe(fd);
    
    if (fork() == 0) {
        close(fd[1]);
        while (read(fd[0], &c, 1) > 0)
            putchar(toupper(c));
        close(fd[0]);
    } else {
        close(fd[0]);
        char *text = "Hello World! Test 123.\n";
        write(fd[1], text, 23);
        close(fd[1]);
        wait(NULL);
    }
    return 0;
}