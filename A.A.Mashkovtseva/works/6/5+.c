#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#define BUFSIZE 500

int main(){
    int fd = open("file.txt", O_RDONLY);
    int terminal = open("/dev/tty", O_RDONLY | O_NDELAY);

    char c;
    long table[2][500];
    int i = 0, j = 0;
    char buf[BUFSIZE];
    
    while(read(fd, &c, 1)) {
        if (c == '\n') {
            j++;
            table[1][i] = j;
            table[0][i] = lseek(fd, 0L, SEEK_CUR);
            j = 0;
            i++;
        }
        else j++;
    }

    printf("Table of line ends and lengths:\n");
    printf("%-5s %-10s %-10s\n", "Idx", "EndPos", "Length");
    for (int k = 0; k < i; ++k) {
        printf("%-5d %-10ld %-10ld\n", k+1, table[0][k], table[1][k]);
    }

    int done = 0;
    while (!done) {
        printf("You have 5 seconds to enter a line number: ");
        fflush(stdout);
        sleep(5);
        int bytes_read = read(terminal, buf, BUFSIZE - 1);
        if (bytes_read <= 0) {
            // Нет ввода - печатаем весь файл
            printf("\nTime's up! Printing entire file:\n");
            lseek(fd, 0L, SEEK_SET);
            int file_bytes;
            while ((file_bytes = read(fd, buf, BUFSIZE)) > 0) {
                write(STDOUT_FILENO, buf, file_bytes);
            }
            close(fd);
            close(terminal);
            done = 1;
            continue;
        } else {
            // Есть ввод - обрабатываем только первое число
            buf[bytes_read] = '\0';
            char *endptr;
            int n = strtol(buf, &endptr, 10);
            // Пропускаем пробелы и переводы строк после числа
            while (*endptr == ' ' || *endptr == '\t' || *endptr == '\n') ++endptr;
            if (buf == endptr || *endptr != '\0') {
                printf("There are only %d rows!\n", i);
                continue;
            }
            if (n <= 0) {
                printf("Exiting...\n");
                close(fd);
                close(terminal);
                done = 1;
                continue;
            }
            if (n > i) {
                printf("There are only %d rows!\n", i);
            } else {
                lseek(fd, table[0][n-1] - table[1][n-1], SEEK_SET);
                char line_buf[table[1][n-1] + 1];
                read(fd, line_buf, table[1][n-1]);
                line_buf[table[1][n-1]] = '\0';
                printf("%s", line_buf);
            }
        }
    }
}