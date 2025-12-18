#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <ctype.h>

typedef struct {
    off_t start;
    off_t size;
} FileRecord;

typedef struct {
    FileRecord *data;
    int total;
    int limit;
} RecordCollection;

void prepareCollection(RecordCollection *rc) {
    rc->data = malloc(sizeof(FileRecord));
    rc->total = 0;
    rc->limit = 1;
}

void addToCollection(RecordCollection *rc, FileRecord rec) {
    if (rc->total == rc->limit) {
        rc->limit *= 2;
        rc->data = realloc(rc->data, rc->limit * sizeof(FileRecord));
    }
    rc->data[rc->total++] = rec;
}

void releaseCollection(RecordCollection *rc) {
    free(rc->data);
    rc->data = NULL;
    rc->total = rc->limit = 0;
}

void displaySummary(RecordCollection *rc) {
    printf("\nFile Line Summary:\n");
    printf("Index | Position | Size\n");
    printf("------+----------+------\n");
    
    for (int i = 0; i < rc->total; i++) {
        printf("%5d | %8ld | %4ld\n", 
               i + 1, 
               rc->data[i].start, 
               rc->data[i].size);
    }
    printf("------+----------+------\n");
    printf("Records: %d\n\n", rc->total);
}

int main(int argc, char *argv[]) {
    if (argc != 2) return 1;
    char *filename = argv[1];

    RecordCollection records;
    prepareCollection(&records);

    int file = open(filename, O_RDONLY);
    if (file == -1) return 1;

    char ch;
    off_t position = 0;
    off_t count = 0;
    while (read(file, &ch, 1) == 1) {
        if (ch == '\n') {
            FileRecord entry = {position, count + 1};
            addToCollection(&records, entry);
            position += count + 1;
            count = 0;
        } else {
            count++;
        }
    }
    if (count > 0) {
        FileRecord final = {position, count};
        addToCollection(&records, final);
    }

    displaySummary(&records);

    fd_set input;
    struct timeval timer;
    
    printf("Input line index: ");
    fflush(stdout);

    FD_ZERO(&input);
    FD_SET(STDIN_FILENO, &input);
    
    timer.tv_sec = 5;
    timer.tv_usec = 0;

    int ready = select(STDIN_FILENO + 1, &input, NULL, NULL, &timer);

    if (ready == 0) {
        printf("Timeout reached!\n");
        lseek(file, 0, SEEK_SET);
        char buffer[1024];
        ssize_t n;
        while ((n = read(file, buffer, sizeof(buffer))) > 0) {
            write(STDOUT_FILENO, buffer, n);
        }
        close(file);
        releaseCollection(&records);
        return 0;
    }

    char user_input[100];
    int value = -1;

    while (1) {
        if (scanf("%99s", user_input) != 1) {
            printf("Read error.\n");
            break;
        }
        
        int valid_number = 1;
        int digits_found = 0;
        
        for (int i = 0; user_input[i] != '\0'; i++) {
            if (i == 0 && user_input[i] == '-') continue;
            if (!isdigit(user_input[i])) {
                valid_number = 0;
                break;
            }
            digits_found = 1;
        }
        
        if (user_input[0] == '-' && !digits_found) valid_number = 0;
        
        if (!valid_number) {
            printf("Numeric input required!\n");
            printf("Input line index: ");
            continue;
        }
        
        value = atoi(user_input);
        
        if (value == 0) break;
        
        if (value < 0) {
            printf("Positive number needed!\n");
            printf("Input line index: ");
            continue;
        }
        
        if (records.total < value) {
            printf("Available: %d record(s).\n", records.total);
            printf("Input line index: ");
            continue;
        }
        
        FileRecord target = records.data[value - 1];
        char *content = calloc(target.size + 1, sizeof(char));
        lseek(file, target.start, SEEK_SET);
        read(file, content, target.size);
        printf("%s", content);
        if (target.size > 0 && content[target.size - 1] != '\n') {
            printf("\n");
        }
        free(content);
        
        printf("Input line index: ");
    }

    close(file);
    releaseCollection(&records);
    return 0;
}