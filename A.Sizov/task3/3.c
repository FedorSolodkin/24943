#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    const char *fname = (argc > 1) ? argv[1] : "data";

    printf("before: real uid=%d  effective uid=%d\n", (int)getuid(), (int)geteuid());

    FILE *file = fopen(fname, "r");

    if (file == NULL)
        perror("fopen");
    else
    {
        fclose(file);
        printf("File was opened and closed successfully.\n");
    }

    setuid(geteuid());

    printf("after: real uid=%d  effective uid=%d\n", (int)getuid(), (int)geteuid());
    if (setuid(getuid()) == -1)
    {
        perror("setuid");
        return 1;
    }

    return 0;
}