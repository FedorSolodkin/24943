#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(){
    int fd = open("file.txt", O_RDONLY);
    

    char c;
    long table[2][500];
    int i = 0, j = 0;
    // table[0][0] = 0L;
    while(read(fd, &c, 1)) {
        if (c == '\n') {
            j++;
            table[1][i] = j;
            table[0][i] = lseek(fd, 0, SEEK_CUR);
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

    char input[100];
    char input[100];
    int n = 1;
    int done = 0;
    while (!done) {
        printf("Enter the row idx or 0 to stop: ");
        if (!fgets(input, sizeof(input), stdin)) break;
        char *endptr;
        n = strtol(input, &endptr, 10);
        while (*endptr == ' ' || *endptr == '\t' || *endptr == '\n') ++endptr;
        if (input == endptr || *endptr != '\0') {
            printf("There are only %d rows!\n", i);
            continue;
        }
        if (n < 0) {
            printf("You should use positive number or 0 to stop!\n");
        } else if (n > i) {
            printf("There are only %d rows!\n", i);
        } else if (n > 0) {
            lseek(fd, table[0][n-1] - table[1][n - 1], SEEK_SET);
            char buf[table[1][n-1] + 1];
            read(fd, buf, table[1][n-1]);
            buf[table[1][n-1]] = '\0';
            printf("%s", buf);
        } else {
            done = 1;
        }
    }
    
    close(fd);
    return 0;
}