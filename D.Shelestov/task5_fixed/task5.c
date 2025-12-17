#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

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
            Line current = {lineOffset, lineLength + 1}; //strlen + '\n'
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

    while (1) {
        int num;
        printf("Enter the line number: ");
        scanf("%d", &num);

        if (num < 0) {
	    printf("The Line number must be positive\n");
            continue;
	}
        if (table.cnt < num) {
            printf("The file contains only %d line(s).\n", table.cnt);
            continue;
        }
	if (num == 0){ break; }

        Line line = table.array[num - 1];
        char *buf = calloc(line.length + 1, sizeof(char));

        lseek(fd, line.offset, SEEK_SET);
        read(fd, buf, line.length * sizeof(char));

        printf("%s\n", buf);
        free(buf);
    }

    close(fd);
    freeArray(&table);

    return 0;
}
