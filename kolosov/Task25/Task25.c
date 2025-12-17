#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

int main(void)
{
    int fd[2];
    pid_t pid;

    pipe(fd);

    pid = fork();

    if (pid == 0) {
        int c;
        close(fd[1]);
        FILE *in = fdopen(fd[0], "r");

        while ((c = fgetc(in)) != EOF) {
            c = toupper((unsigned char)c);
            putchar(c);
        }

        fclose(in);
    } else {
        close(fd[0]);
        FILE *out = fdopen(fd[1], "w");

        fputs("TeST texT This is a TeSt.\n", out);

        fclose(out);
        wait(NULL);
    }

    return 0;
}
