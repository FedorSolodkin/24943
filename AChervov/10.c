#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "No args provided.\n");
    }
    pid_t pid = fork();
    if (pid == -1)
    {
        fprintf(stderr, "Couldn't create a new process.");
        exit(1);
    }

    if (pid)
    {
        int status;
        wait(&status);
        printf("The program '%s' returned: %d\n", argv[1], status);
    }
    else
    {
        execvp(argv[1], (char* const*)&argv[1]);
    }
    
    
    return 0;
}
