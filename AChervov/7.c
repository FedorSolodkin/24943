#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

void termination_handler(int signum);

int fd;
long offsets[256];
int lnlens[256];
int lines = 0;
int buf;
int terminated = 0;

struct sigaction alarm_act;

int main(int argc, char *argv[])
{

    // File open

    fd = open(argv[1], O_RDONLY);
    if (fd == -1)
    {
        perror("Error opening file.");
        exit(1);
    }
    struct stat fst;
    fstat(fd, &fst);
    size_t flength = fst.st_size;

    char *map = mmap(NULL, flength, PROT_READ, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED)
    {
        perror("`mmap` error!");
        exit(1);
    }


    // Tables

    offsets[0] = 0;
    alarm_act.sa_handler = termination_handler;

    // alarm_act.__sigaction_handler.sa_handler = termination_handler;

    // int pos = 0;
    char *ptr = map;
    int lnlen = 0;
    while (ptr < (map + flength))
    {
        ptr++;
        if (*ptr == '\n')
        {
            lnlens[lines] = lnlen;
            offsets[++lines] = ptr - map;
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
    
    int input;
    printf("Enter the line number [0-%d]: ", lines-1);
    sigaction(SIGALRM, &alarm_act, NULL);
    unsigned int x = alarm(5);
    scanf("%d", &input);
    if (terminated)
    {
        return 0;
    }

    if (input >= lines || input < 0)
    {
        fprintf(stderr, "Line index out of bounds.\n");
        return -1;
    }
    
    // lseek(fd, offsets[input], SEEK_SET);
    ptr = map + offsets[input];

    char buff[256];
    int idx = 0;
    while (ptr < (map + flength) && *ptr != '\n')
    {
        buff[idx++] = *(ptr++);
    }
    buff[idx] = '\0';
    // for (int i = 0; read(fd, &(buff[i]), 1) > 0 && buff[i] != '\n'; i++);

    printf("%s\n", buff);

    // Map deinit
    int ret = munmap(map, flength);
    if (ret == -1)
    {
        perror("`munmap` error!");
        exit(1);
    }
    
    return 0;
}

void termination_handler(int signum)
{
    if (signum == SIGALRM)
    {
        putc('\n', stdout);
        lseek(fd, 0, SEEK_SET);
        while (read(fd, &buf, 1) > 0)
        {
            putc(buf, stdout);
        }
        putc('\n', stdout);
        terminated = 1;
    }
}
