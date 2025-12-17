#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

static int timeout_occurred = 0;

void alarm_handler(int sig) {
    timeout_occurred = 1;
}

int main(int argc, char* argv[]) {
    if (argc != 2) return 1;

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) return 1;

    off_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    char* buf = malloc(size);
    if (size > 0) {
        read(fd, buf, size);
    }

    off_t offsets[1000];
    int lines = 0;
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

    signal(SIGALRM, alarm_handler);

    int n;
    while (1) {
        timeout_occurred = 0;
        alarm(5);

        printf("Line: ");
        fflush(stdout);

        if (scanf("%d", &n) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            n = -1;
        }

        alarm(0);

        if (timeout_occurred) {
            lseek(fd, 0, SEEK_SET);
            if (size > 0) {
                char* full = malloc(size);
                read(fd, full, size);
                write(STDOUT_FILENO, full, size);
                if (size > 0 && full[size - 1] != '\n') {
                    write(STDOUT_FILENO, "\n", 1);
                }
                free(full);
            }
            break;
        }

        if (n == 0) {
            break;
        }

        if (n < 1 || n > lines) {
            printf("Invalid\n");
            continue;
        }

        off_t start = offsets[n - 1];
        off_t end = (n < lines) ? offsets[n] : size;
        lseek(fd, start, SEEK_SET);
        char out[end - start];
        read(fd, out, end - start);
        write(STDOUT_FILENO, out, end - start);
        if (end - start == 0 || out[end - start - 1] != '\n') {
            write(STDOUT_FILENO, "\n", 1);
        }
    }

    close(fd);
    free(buf);
    return 0;
}
