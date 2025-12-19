#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/wait.h>

char buf[128];

int main(int argc, char const *argv[])
{
    int fd[2];
    if (pipe(fd) == -1)
    {
        printf("Pipe creation error.");
        exit(1);
    }

    pid_t pid = fork();

    if (pid > 0)
    {
        close(fd[0]);
        while (1)
        {
            int sz = read(STDIN_FILENO, buf, sizeof(buf));
            if (sz == 0) break;
            write(fd[1], buf, sz);
        }
        close(fd[1]);
        wait(NULL);
    }
    else if (pid == 0)
    {
        close(fd[1]);
        while (1)
        {
            int sz = read(fd[0], buf, sizeof(buf));
            if (sz == 0) break;
            for (int i = 0; i < sz; i++)
                buf[i] = toupper(buf[i]);
            write(STDOUT_FILENO, buf, sz);
        }
        close(fd[0]);
    }
    else
    {
        printf("Couldn't fork process.");
        exit(1);
    }

    return 0;
}
