#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wordexp.h>

void open_test(const char *path)
{
    // char fullpath[200];
    // strcpy(fullpath, getenv("HOME"));
    // strcat(fullpath, "/restr");
    printf("%s\n", path);
    FILE *restr_f = fopen(path, "r");
    if (restr_f == NULL)
    {
        fprintf(stderr, "Couldn't open file\n");
    }
    else
    {
        printf("Successfull open.\n");
        fclose(restr_f);
    }
}

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: ./this_program \"path/to/file\"\n");
        return -1;
    }
    printf("Testing with effective UID:\n");
    printf("UID: %d, EUID: %d\n", getuid(), geteuid());
    open_test(argv[1]);
    printf("\nTesting with real UID:\n");
    setuid(getuid());
    printf("UID: %d, EUID: %d\n", getuid(), geteuid());
    open_test(argv[1]);
    
    return 0;
}
