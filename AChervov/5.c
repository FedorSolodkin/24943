#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#define MNO 1

long offsets[256];
int lnlens[256];
int lines = 0;
int buf;

int main(int argc, char *argv[])
{
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1)
    {
        perror("Error opening file.");
        exit(1);
    }
    offsets[0] = 0;

    int pos = 0;
    int lnlen = 0;
    while (read(fd, &buf, 1) > 0)
    {
        pos++;
        if (buf == '\n')
        {
            lnlens[lines] = lnlen;
            offsets[++lines] = pos;
            lnlen = 0;
        }
        else lnlen++;
    }
    lnlens[lines] = lnlen;
    lines++;


    /* Debug */
    // for (int i = 0; i < lines; i++)
    // {
    //     printf("%ld %d\n", offsets[i], lnlens[i]);
    // }

    printf("Argc %d\n", argc);
    if (argc > 2 && !strcmp(argv[2], "-d"))
    {
        for (size_t i = 0; i < lines; i++)
        {
            printf("Line #%ld, Offset=%ld, Length=%d\n", i+1, offsets[i], lnlens[i]);
        }
        exit(0);
    }
    
    int input = -1;
    while (input)
    {
        printf("Enter the line number [1-%d] (or 0 to exit): ", lines);
        if (scanf("%d", &input) == 0)
        {
            fprintf(stderr, "Incorrect line input.\n");
            while ((input = getchar()) != '\n' && input != EOF);
            input = -1;
            continue;
        }
        else if (input > lines || input < 0)
        {
            fprintf(stderr, "Line index out of bounds.\n");
            continue;
        }
        else if (input == 0)
        {
            exit(0);
        }

        lseek(fd, offsets[input-1], SEEK_SET);

        char buff[256];
        int idx = 0;
        while (read(fd, &(buff[idx]), 1) > 0 && buff[idx] != '\n')
        {
            idx++;
        }
        buff[idx] = '\0';
        // for (int i = 0; read(fd, &(buff[i]), 1) > 0 && buff[i] != '\n'; i++);

        printf("%s\n", buff);
    }
    
    
    
    return 0;
}
