#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

volatile int timeout = 0;

void alarm_handler(int sig) {
    timeout = 1;
}

void print_file(int f){
    char buffer[256];
    ssize_t bytes_read;
    
    printf("\n=== Timeout! Printing entire file content ===\n");
    

    lseek(f, 0, SEEK_SET);

    while ((bytes_read = read(f, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }
    printf("\n=== End of file ===\n");
}

typedef struct{
    off_t offset;
    size_t len;
} Line;

typedef struct{
    Line * array;
    int count;
    int capacity;
} Array;

void init(Array * mas){
    mas->array = malloc(sizeof(Line));
    mas->count = 0;
    mas->capacity = 1;
}

void add_line(Array * mas, Line el){
    if (mas->count == mas->capacity){
        mas->capacity *= 2;
        mas->array = realloc(mas->array, mas->capacity * sizeof(Line));
    }
    mas->array[mas->count++] = el;
}


int main(int argc, char ** argv){
    if (argc < 2) {
        printf("Filename is not specified\n");
        return 1;
    }

    signal(SIGALRM, alarm_handler);

    Array mas;
    init(&mas);

    int f = open(argv[1], O_RDONLY);
    char c;

    off_t offset_len = 0;
    size_t len = 0;

    while(read(f, &c, 1) == 1){
        if (c == '\n'){
            Line el = {offset_len, len};
            add_line(&mas, el);
            offset_len += len + 1;
            len = 0;
        } else{
            len++;
        }
    }

    if (len > 0) {
        Line el = {offset_len, len};
        add_line(&mas, el);
    }

    int line_number;
    char buffer[256];

    printf("\nYou have 5 seconds to enter line number (1-%d, 0 to exit): ", mas.count);
    fflush(stdout);
    
    alarm(5); 
    
    int result = scanf("%d", &line_number);
    
    alarm(0);
    if (timeout) {
        print_file(f);
    } else{
        if (line_number == 0){
            printf("Exit\n");
        }
    
        if (line_number < 1 || line_number > mas.count) {
            printf("Line number must be between 1 and %d\n", mas.count);
        } else{
            Line *line_info = &mas.array[line_number - 1];
        
            if (lseek(f, line_info->offset, SEEK_SET) == -1) {
                perror("Error seeking to line");
            }else{
                ssize_t bytes_read = read(f, buffer, line_info->len);

                buffer[bytes_read] = '\0';
                printf("Line %d: %s\n", line_number, buffer);
            }

        }
    }

    if (result != 1) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }




    while (1 && !timeout && line_number != 0){
        printf("\nEnter line number (1-%d, 0 to exit): ", mas.count);
        if (scanf("%d", &line_number) != 1) {
            printf("invalid input\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }


        if (line_number == 0){
	    printf("Exit\n");
	    break;
        }
        
        if (line_number < 1 || line_number > mas.count) {
            printf("Line number must be between 1 and %d\n", mas.count);
            continue;
        }

        Line *line_info = &mas.array[line_number - 1];
        

        if (lseek(f, line_info->offset, SEEK_SET) == -1) {
            perror("Error seeking to line");
            continue;
        }

        ssize_t bytes_read = read(f, buffer, line_info->len);

        buffer[bytes_read] = '\0';
        printf("Line %d: %s\n", line_number, buffer);
    }


    free(mas.array);
    close(f);
    return 0;
}
