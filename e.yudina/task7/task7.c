#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

static int timeout_occurred = 0;

void alarm_handler(int sig) {
    timeout_occurred = 1;
}

int main(int argc, char* argv[]) {
    if (argc != 2) return 1;

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) return 1;

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        close(fd);
        return 1;
    }

    off_t size = sb.st_size;

    char* mapped = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        close(fd);
        return 1;
    }

    off_t offsets[1000];
    int lines = 0;
    offsets[lines++] = 0;

    for (off_t i = 0; i < size; i++) {
        if (mapped[i] == '\n') {
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
            fwrite(mapped, 1, size, stdout);
            if (size > 0 && mapped[size - 1] != '\n') {
                putchar('\n');
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
        off_t len = (n < lines ? offsets[n] : size) - start;

        fwrite(mapped + start, 1, len, stdout);
        if (len == 0 || mapped[start + len - 1] != '\n') {
            putchar('\n');
        }
    }

    munmap(mapped, size);
    close(fd);
    return 0;
}
