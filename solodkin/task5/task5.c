#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct {
    long pos;
    long len;
} LineInfo;

typedef struct {
    LineInfo *items;
    int count;
    int max;
} LineStorage;

void setupStorage(LineStorage *store) {
    store->items = (LineInfo *)malloc(sizeof(LineInfo));
    store->count = 0;
    store->max = 1;
}

void storeLine(LineStorage *store, LineInfo info) {
    if (store->count >= store->max) {
        store->max *= 2;
        store->items = (LineInfo *)realloc(store->items, store->max * sizeof(LineInfo));
    }
    store->items[store->count++] = info;
}

void clearStorage(LineStorage *store) {
    free(store->items);
    store->items = NULL;
    store->count = 0;
    store->max = 0;
}

void showLines(LineStorage *store) {
    printf("\n=== File Lines ===\n");
    printf("Line  | Position | Length\n");
    printf("------+----------+-------\n");
    
    for (int i = 0; i < store->count; i++) {
        printf("%5d | %8ld | %6ld\n", 
               i + 1, 
               store->items[i].pos, 
               store->items[i].len);
    }
    printf("------+----------+-------\n");
    printf("Total: %d lines\n\n", store->count);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];
    LineStorage storage;
    setupStorage(&storage);

    int file = open(filename, O_RDONLY);
    if (file < 0) {
        perror("File error");
        clearStorage(&storage);
        return 1;
    }

    long curr = 0;
    long line = 0;
    char ch;

    while (read(file, &ch, 1) > 0) {
        if (ch == '\n') {
            LineInfo newline = {curr, line + 1};
            storeLine(&storage, newline);
            
            curr += line + 1;
            line = 0;
        } else {
            line++;
        }
    }

    if (line > 0) {
        LineInfo last = {curr, line};
        storeLine(&storage, last);
    }

    showLines(&storage);

    while (1) {
        int num;
        printf("Enter line number (0 to quit): ");
        
        if (scanf("%d", &num) != 1) {
            printf("Invalid input.\n");
            while (getchar() != '\n');
            continue;
        }

        if (num == 0) {
            break;
        }

        if (num < 0) {
            printf("Must be positive.\n");
            continue;
        }

        if (num > storage.count) {
            printf("File has only %d lines.\n", storage.count);
            continue;
        }

        LineInfo target = storage.items[num - 1];
        char *buf = (char *)calloc(target.len + 1, sizeof(char));

        if (buf) {
            lseek(file, target.pos, SEEK_SET);
            read(file, buf, target.len);
            
            printf("Line %d: %s\n", num, buf);
            free(buf);
        }
    }

    close(file);
    clearStorage(&storage);

    return 0;
}