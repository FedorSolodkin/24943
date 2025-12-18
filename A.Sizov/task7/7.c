#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/select.h>

#define MAXLINES 1000

int main(int argc, char *argv[]) {
    int fd = open(argv[1], O_RDONLY);
    struct stat st;
    fstat(fd, &st);
    size_t filesize = st.st_size;

    char *data = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    long offsets[MAXLINES];
    size_t lengths[MAXLINES];
    int line_count = 1;
    offsets[0] = 0;

    for (size_t i = 0; i < filesize; i++) {
        if (data[i] == '\n') {
            if (line_count < MAXLINES)
                offsets[line_count++] = i + 1;
        }
    }

    for (int i = 0; i < line_count - 1; i++)
        lengths[i] = offsets[i + 1] - offsets[i];
    lengths[line_count - 1] = filesize - offsets[line_count - 1];

    while (1) {
        printf("Enter line number (0 to quit): ");
        fflush(stdout);

        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        struct timeval tv = {5, 0};

        int r = select(1, &rfds, NULL, NULL, &tv);
        if (r == 0) {
            printf("\nTimeout. Printing entire file:\n");
            fwrite(data, 1, filesize, stdout);
            break;
        }

        int num;
        scanf("%d", &num);
        if (num == 0) break;

        size_t len = lengths[num - 1];
        const char *line_start = data + offsets[num - 1];

        if (len > 0 && line_start[len - 1] == '\n') len--;
        fwrite(line_start, 1, len, stdout);
        printf("\n");
    }

    munmap(data, filesize);
    return 0;
}
