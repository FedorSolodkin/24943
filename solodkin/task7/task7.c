#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <sys/stat.h>

typedef struct {
    off_t pos;
    off_t len;
} Segment;

typedef struct {
    Segment *items;
    int total;
    int capacity;
} Collection;

void initialize(Collection *c) {
    c->items = malloc(sizeof(Segment));
    c->total = 0;
    c->capacity = 1;
}

void addSegment(Collection *c, Segment s) {
    if (c->total == c->capacity) {
        c->capacity *= 2;
        c->items = realloc(c->items, c->capacity * sizeof(Segment));
    }
    c->items[c->total++] = s;
}

void cleanup(Collection *c) {
    free(c->items);
    c->items = NULL;
    c->total = c->capacity = 0;
}

void displayInfo(Collection *c) {
    printf("\nSegment Information:\n");
    printf("Index  | Position |  Size\n");
    printf("-------+----------+-------\n");
    
    for (int i = 0; i < c->total; i++) {
        printf("%6d | %8ld | %6ld\n", 
               i + 1, 
               c->items[i].pos, 
               c->items[i].len);
    }
    printf("-------+----------+-------\n");
    printf("Total segments: %d\n\n", c->total);
}

int main(int argc, char *argv[]) {
    if (argc != 2) return 1;
    
    char *filename = argv[1];
    Collection segments;
    initialize(&segments);

    int file = open(filename, O_RDONLY);
    if (file == -1) return 1;

    struct stat fileinfo;
    if (fstat(file, &fileinfo) == -1) {
        close(file);
        return 1;
    }
    off_t filesize = fileinfo.st_size;

    char *memory = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, file, 0);
    if (memory == MAP_FAILED) {
        close(file);
        return 1;
    }

    off_t current = 0;
    off_t count = 0;
    
    for (off_t i = 0; i < filesize; i++) {
        if (memory[i] == '\n') {
            Segment entry = {current, count + 1};
            addSegment(&segments, entry);
            current += count + 1;
            count = 0;
        } else {
            count++;
        }
    }
    
    if (count > 0) {
        Segment final = {current, count};
        addSegment(&segments, final);
    }

    displayInfo(&segments);

    fd_set inputs;
    struct timeval timer;
    
    printf("Provide segment index: ");
    fflush(stdout);

    FD_ZERO(&inputs);
    FD_SET(STDIN_FILENO, &inputs);
    
    timer.tv_sec = 5;
    timer.tv_usec = 0;

    int ready = select(STDIN_FILENO + 1, &inputs, NULL, NULL, &timer);

    if (ready == 0) {
        printf("Timeout occurred!\n");
        fwrite(memory, 1, filesize, stdout);
        munmap(memory, filesize);
        close(file);
        cleanup(&segments);
        return 0;
    }

    int index;
    scanf("%d", &index);

    while (index != 0) {
        if (index < 0) {
            printf("Index must be positive!");
        } else if (segments.total < index) {
            printf("Available segments: %d\n", segments.total);
        } else {
            Segment target = segments.items[index - 1];
            fwrite(memory + target.pos, 1, target.len, stdout);
            if (target.len > 0 && memory[target.pos + target.len - 1] != '\n') {
                printf("\n");
            }
        }
        
        printf("Provide segment index: ");
        scanf("%d", &index);
    }

    munmap(memory, filesize);
    close(file);
    cleanup(&segments);
    
    return 0;
}