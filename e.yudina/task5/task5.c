#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    if (argc != 2) return 1;

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) return 1;

    off_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    char* buf = malloc(size);
    read(fd, buf, size);

    off_t offsets[1000];
    int lines = 0, pos = 0;
    offsets[lines++] = 0;

    for (int i = 0; i < size; i++) {
        if (buf[i] == '\n') {
            offsets[lines++] = i + 1;
        }
    }

    for (int i = 0; i < lines; i++) {
        off_t len = (i + 1 < lines ? offsets[i + 1] : size) - offsets[i];
        printf("%d\t%ld\t%ld\n", i + 1, (long)offsets[i], (long)len);
    }

    int n;
    while (printf("Line: "), scanf("%d", &n), n != 0) {
        if (n < 1 || n > lines) {
            printf("Invalid\n");
            continue;
        }
        off_t start = offsets[n - 1];
        off_t end = (n < lines) ? offsets[n] : size;
        lseek(fd, start, SEEK_SET);
        char out[end - start];
        read(fd, out, end - start);
        write(1, out, end - start);
        if (out[end - start - 1] != '\n') write(1, "\n", 1);
    }

    close(fd);
    free(buf);
    return 0;
}
