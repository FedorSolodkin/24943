#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void check_ids_and_open_file()
{
    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());

    const char* filename = "task3.txt";
    printf("Try to open file \"%s\"\n", filename);
    FILE *file = fopen(filename, "r");

    if (file == NULL)
        perror("fopen");
    else
    {
        fclose(file);
        printf("File was opened and closed successfully.\n");
    }
}

int main(void)
{
    printf("===== Try without SUID: =====\n");
    check_ids_and_open_file();

    printf("======= Try with SUID: ======\n");
    if (setuid(getuid()) == -1)
    {
        perror("setuid");
        return 1;
    }
    check_ids_and_open_file();

    return 0;
}
