#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <sys/stat.h>

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

    // file size
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        close(fd);
        return 1;
    }
    off_t file_size = sb.st_size;

    // show file in mem
    char *file_data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_data == MAP_FAILED) {
        close(fd);
        return 1;
    }

    // table of strings
    off_t lineOffset = 0;
    off_t lineLength = 0;
    for (off_t i = 0; i < file_size; i++) {
        if (file_data[i] == '\n') {
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
        // file out
        fwrite(file_data, 1, file_size, stdout);
        munmap(file_data, file_size);
        close(fd);
        freeArray(&table);
        return 0;
    }

    int num;
    scanf("%d", &num);

    while (num != 0) {
        if (num < 0) {
            printf("Line number must be positive!");
        } else if(table.cnt < num){
            printf("The file contains only %d line(s).\n", table.cnt);
        } else {
            Line line = table.array[num - 1];
            // string out from image memory
            fwrite(file_data + line.offset, 1, line.length, stdout);
            if (line.length > 0 && file_data[line.offset + line.length - 1] != '\n') {
                printf("\n");
            }
        }
        
        printf("Enter the line number: ");
        scanf("%d", &num);
    }

    munmap(file_data, file_size);
    close(fd);
    freeArray(&table);
    return 0;
}
