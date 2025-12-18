#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define MAXLINES 1000
#define BUFSIZE 4096

int main(int argc, char *argv[])
{
    int fd;
    off_t offsets[MAXLINES];
    size_t lengths[MAXLINES];
    int line_count = 0;
    char buf[BUFSIZE];
    ssize_t n;

    if (argc < 2) {
        printf("Usage: %s file\n", argv[0]);
        return 1;
    }

    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    offsets[line_count++] = 0; // первая строка с нулевого смещения
    off_t pos = 0;

    while ((n = read(fd, buf, BUFSIZE)) > 0) {
        for (ssize_t i = 0; i < n; i++) {
            if (buf[i] == '\n' && line_count < MAXLINES) {
                offsets[line_count++] = pos + i + 1;
            }
        }
        pos += n;
    }

    off_t file_end = pos; // конец файла

    for (int i = 0; i < line_count - 1; i++)
        lengths[i] = offsets[i + 1] - offsets[i];
    lengths[line_count - 1] = file_end - offsets[line_count - 1];

    // печать таблицы
    for (int i = 0; i < line_count; i++)
        printf("Line %3d: Offset = %8lld, Length = %5zu\n",
               i + 1, (long long)offsets[i], lengths[i]);
    printf("-----------------------------------\n\n");

    while (1) {
        int num;
        printf("Enter line number (0 to quit): ");
        if (scanf("%d", &num) != 1 || num == 0) break;

        if (num < 1 || num > line_count) {
            printf("Invalid line number\n");
            continue;
        }

        lseek(fd, offsets[num - 1], SEEK_SET);
        char *line = malloc(lengths[num - 1] + 1);
        read(fd, line, lengths[num - 1]);
        line[lengths[num - 1]] = '\0';

        if (lengths[num - 1] > 0 && line[lengths[num - 1] - 1] == '\n')
            line[lengths[num - 1] - 1] = '\0';

        printf("%s\n", line);
        free(line);
    }

    close(fd);
    return 0;
}
