#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <ctype.h>

typedef struct {
    off_t offset;
    off_t length;
} Line;

typedef struct {
    Line *array;
    int cnt;
    int cap;
} Array;

void initArray(Array *a) {
    a->array = malloc(sizeof(Line));
    a->cnt = 0;
    a->cap = 1;
}

void insertArray(Array *a, Line element) {
    if (a->cnt == a->cap) {
        a->cap *= 2;
        a->array = realloc(a->array, a->cap * sizeof(Line));
    }
    a->array[a->cnt++] = element;
}

void freeArray(Array *a) {
    free(a->array);
    a->array = NULL;
    a->cnt = a->cap = 0;
}

void printTable(Array *table) {
    printf("\nLine Table (Offset and Length):\n");
    printf("Line #  |  Offset  |  Length\n");
    printf("--------+----------+---------\n");
    
    for (int i = 0; i < table->cnt; i++) {
        printf("%6d  | %8ld | %7ld\n", 
               i + 1, 
               table->array[i].offset, 
               table->array[i].length);
    }
    printf("--------+----------+---------\n");
    printf("Total lines: %d\n\n", table->cnt);
}

int main(int argc, char *argv[]) {
    if (argc != 2) { return 1; }
    char *path = argv[1];

    Array table;
    initArray(&table);

    int fd = open(path, O_RDONLY);
    if (fd == -1) { return 1; }

    char c;
    off_t lineOffset = 0;
    off_t lineLength = 0;
    while (read(fd, &c, 1) == 1) {
        if (c == '\n') {
            Line current = {lineOffset, lineLength + 1};
            insertArray(&table, current);
            lineOffset += lineLength + 1;
            lineLength = 0;
        } else {
            lineLength++;
        }
    }
    if (lineLength > 0) {
        Line current = {lineOffset, lineLength};
        insertArray(&table, current);
    }

    printTable(&table);

    fd_set readfds;
    struct timeval timeout;
    
    printf("Enter the line number: ");
    fflush(stdout);

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    int retval = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);

    if (retval == 0) {
        printf("The time is up!\n");
        lseek(fd, 0, SEEK_SET);
        char buf[1024];
        ssize_t bytes;
        while ((bytes = read(fd, buf, sizeof(buf))) > 0) {
            write(STDOUT_FILENO, buf, bytes);
        }
        close(fd);
        freeArray(&table);
        return 0;
    }

    char input[100];
    int num = -1;

    while (1) {
        if (scanf("%99s", input) != 1) {
            printf("Error reading input.\n");
            break;
        }
        
        int is_number = 1;
        int has_digits = 0;
        
        for (int i = 0; input[i] != '\0'; i++) {
            if (i == 0 && input[i] == '-') {
                continue;
            }
            if (!isdigit(input[i])) {
                is_number = 0;
                break;
            }
            has_digits = 1;
        }
        
        if (input[0] == '-' && !has_digits) {
            is_number = 0;
        }
        
        if (!is_number) {
            printf("Input must be a number!\n");
            printf("Enter the line number: ");
            continue;
        }
        
        num = atoi(input);
        
        if (num == 0) {
            break;
        }
        
        if (num < 0) {
            printf("Line number must be positive!\n");
            printf("Enter the line number: ");
            continue;
        }
        
        if (table.cnt < num) {
            printf("The file contains only %d line(s).\n", table.cnt);
            printf("Enter the line number: ");
            continue;
        }
        
        Line line = table.array[num - 1];
        char *buf = calloc(line.length + 1, sizeof(char));
        lseek(fd, line.offset, SEEK_SET);
        read(fd, buf, line.length);
        printf("%s", buf);
        if (buf[line.length - 1] != '\n') {
            printf("\n");
        }
        free(buf);
        
        printf("Enter the line number: ");
    }

    close(fd);
    freeArray(&table);
    return 0;
}
