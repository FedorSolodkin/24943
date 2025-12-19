#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>

#define BUFSIZE 500

int main(){
    int fd = open("file.txt", O_RDONLY);
    
    int terminal = open("/dev/tty", O_RDONLY | O_NDELAY);

    off_t size = lseek(fd, 0, SEEK_END);
    
    // Отображаем файл в память
    char *p = (char*)mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
    
    
    long table[2][500];
    int i = 0, j = 0;
 
    table[0][0] = 0;
    
    for(int count = 0; count < size; count++) {
        if (p[count] == '\n') {
            j++;
            table[1][i] = j;
            table[0][i] = count + 1;
            i++;
            j = 0;
        }
        else j++;
    }

    printf("Table of line ends and lengths:\n");
    printf("%-5s %-10s %-10s\n", "Idx", "EndPos", "Length");
    for (int k = 0; k < i; ++k) {
        printf("%-5d %-10ld %-10ld\n", k+1, table[0][k], table[1][k]);
    }
    
    int num_lines = i; 
    
    char buf[BUFSIZE];
    
    int done = 0;
    while (!done) {
        printf("You have 5 seconds to enter a line number: ");
        fflush(stdout);
        sleep(5);
        int bytes_read = read(terminal, buf, BUFSIZE - 1);
        if (bytes_read <= 0) {
            printf("\nTime's up! Printing entire file:\n");
            write(STDOUT_FILENO, p, size);  // Используем mmap вместо read/write
            munmap(p, size);
            close(fd);
            close(terminal);
            done = 1;
            continue;
        } else {
            buf[bytes_read] = '\0';
            char *endptr;
            int n = strtol(buf, &endptr, 10);
            while (*endptr == ' ' || *endptr == '\t' || *endptr == '\n') ++endptr;
            if (buf == endptr || *endptr != '\0') {
                printf("There are only %d rows!\n", num_lines);
                continue;
            }
            if (n <= 0) {
                printf("Exiting...\n");
                munmap(p, size);
                close(fd);
                close(terminal);
                done = 1;
                continue;
            }
            if (n > num_lines) {
                printf("There are only %d rows!\n", num_lines);
            } else {
                int length = table[1][n-1]; 
                long end_offset = table[0][n-1];
                long start_offset = end_offset - length; // Вычисляем начало строки
                write(STDOUT_FILENO, p + start_offset, length);
            }
        }
    }
}