#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>



int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "No path provided.\n");
        exit(1);
    }

    pid_t pid = fork();

    if (pid == 0)
    {
        execlp("cat", "cat", argv[1], NULL);
    }
    else
    {
        wait(NULL);
        printf("Text.\n");
    }
    return 0;
}
