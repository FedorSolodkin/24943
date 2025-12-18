#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

#define MAXLINES 1000
#define BUFSIZE 4096

void print_all(int fd) {
    if (lseek(fd, 0, SEEK_SET) < 0) {
        perror("lseek");
        return;
    }

    char buf[BUFSIZE];
    ssize_t n;
    while ((n = read(fd, buf, BUFSIZE)) > 0) {
        if (write(STDOUT_FILENO, buf, n) != n) {
            perror("write");
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        write(2, "Usage: ./program filename\n", 26);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    long offsets[MAXLINES];
    size_t lengths[MAXLINES];
    int line_count = 0;

    offsets[0] = 0;
    line_count = 1;

    char buf[BUFSIZE];
    ssize_t n;
    long total_read = 0;

    while ((n = read(fd, buf, BUFSIZE)) > 0) {
        for (ssize_t i = 0; i < n; i++) {
            if (buf[i] == '\n') {
                if (line_count >= MAXLINES) {
                    fprintf(stderr, "Too many lines (max %d)\n", MAXLINES);
                    close(fd);
                    return 1;
                }
                offsets[line_count++] = total_read + i + 1;
            }
        }
        total_read += n;
    }

    off_t file_size = lseek(fd, 0, SEEK_END);

    for (int i = 0; i < line_count - 1; i++) {
        lengths[i] = offsets[i + 1] - offsets[i];
    }
    lengths[line_count - 1] = file_size - offsets[line_count - 1];

    // Вывод таблицы для отладки
    printf("Line count: %d\n", line_count);
    printf("Line#   Offset    Length\n");
    for (int i = 0; i < line_count; i++) {
        printf("%5d %9ld %7zu\n", i + 1, offsets[i], lengths[i]);
    }
    printf("\n");

    int num;
    char *line_buf;

    // Первый запрос — с таймаутом 5 секунд
    printf("Enter line number (0 to quit): ");
    fflush(stdout);

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);
    struct timeval tv = {5, 0}; // 5 секунд

    int ret = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);
    if (ret == -1) {
        perror("select");
        close(fd);
        return 1;
    } else if (ret == 0) {
        printf("\nTimeout. Printing entire file:\n");
        print_all(fd);
        close(fd);
        return 0;
    }

    // Если ввёл — читаем номер
    if (scanf("%d", &num) != 1) {
        fprintf(stderr, "Invalid input\n");
        close(fd);
        return 1;
    }
    if (num == 0) {
        close(fd);
        return 0;
    }

    while (1) {
        if (num < 1 || num > line_count) {
            printf("Line number out of range (1-%d)\n", line_count);
        } else {
            if (lseek(fd, offsets[num - 1], SEEK_SET) < 0) {
                perror("lseek");
                break;
            }

            line_buf = malloc(lengths[num - 1] + 1);
            if (!line_buf) {
                perror("malloc");
                break;
            }

            ssize_t r = read(fd, line_buf, lengths[num - 1]);
            if (r < 0) {
                perror("read");
                free(line_buf);
                break;
            }
            line_buf[r] = '\0';

            if (r > 0 && line_buf[r - 1] == '\n') {
                line_buf[r - 1] = '\0';
            }

            printf("%s\n", line_buf);
            free(line_buf);
        }

        printf("Enter line number (0 to quit): ");
        fflush(stdout);

        if (scanf("%d", &num) != 1) {
            fprintf(stderr, "Invalid input\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        if (num == 0) break;
    }

    close(fd);
    return 0;
}
