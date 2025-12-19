#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define MAX_LINES   1000
#define BUFFER_SIZE 1024
#define TIMEOUT     5

typedef struct {
    off_t  offset;  
    size_t length;   
} line_info_t;

volatile sig_atomic_t timeout_occurred = 0;
char *filename_global = NULL;

void alarm_handler(int sig) {
    timeout_occurred = 1;
}

void print_entire_file(const char *filename) {
    printf("\nTimeout: no input within %d seconds.\n", TIMEOUT);
    printf("========================================\n");
    fflush(stdout);
    
    pid_t pid = fork();
    
    if (pid == -1) {
        perror("fork");
        return;
    }
    
    if (pid == 0) {
        execlp("cat", "cat", filename, (char *)NULL);
        perror("execlp cat");
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);
        printf("========================================\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    filename_global = argv[1];

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    size_t file_size = st.st_size;

    // Отображение файла в память
    char *mapped_mem = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped_mem == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    line_info_t lines[MAX_LINES];
    int line_count = 0;
    off_t line_start = 0;

    for (off_t i = 0; i < file_size; i++) {
        if (mapped_mem[i] == '\n') {
            if (line_count < MAX_LINES) {
                lines[line_count].offset = line_start;
                lines[line_count].length = i - line_start;
                line_count++;
            }
            line_start = i + 1;
        }
    }

    if (line_start < file_size && line_count < MAX_LINES) {
        lines[line_count].offset = line_start;
        lines[line_count].length = file_size - line_start;
        line_count++;
    }

    printf("\nLine table for file '%s':\n", argv[1]);
    printf("Line#\tOffset\tLength\n");
    printf("-----\t------\t------\n");
    for (int i = 0; i < line_count; i++) {
        printf("%d\t%ld\t%zu\n",
               i + 1,
               (long)lines[i].offset,
               lines[i].length);
    }
    printf("Total lines: %d\n\n", line_count);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = alarm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction");
        munmap(mapped_mem, file_size);
        close(fd);
        return 1;
    }

    int input_received = 0;
    
    while (1) {
        char input[64];
        int line_number;

        // Если пользоватль уже вводил данные, таймаут не применяется
        if (!input_received) {
            timeout_occurred = 0;
            alarm(TIMEOUT);
        }

        printf("Enter line number (0 to exit): ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) {
            alarm(0);
            if (timeout_occurred && !input_received) {
                print_entire_file(filename_global);
                break;
            }
            printf("\nProcess_end (EOF)\n");
            break;
        }

        // Ввод получен
        if (!input_received) {
            alarm(0);
            
            if (timeout_occurred) {
                print_entire_file(filename_global);
                break;
            }
            
            input_received = 1;  // Отмечаем, что пользователь начал вводить данные
        }

        input[strcspn(input, "\n")] = '\0';

        char *endptr;
        line_number = (int)strtol(input, &endptr, 10);

        if (input[0] == '\0' || endptr == input || *endptr != '\0') {
            printf("Invalid input. Please enter a number.\n");
            continue;
        }

        if (line_number == 0) {
            printf("Process_end\n");
            break;
        }

        if (line_number < 1 || line_number > line_count) {
            printf("Line number must be between 1 and %d\n", line_count);
            continue;
        }

        line_info_t *line = &lines[line_number - 1];

        printf("Line %d: ", line_number);
        fwrite(mapped_mem + line->offset, 1, line->length, stdout);
        printf("\n");
    }

    alarm(0);
    munmap(mapped_mem, file_size);  // Освобождение отображенной памяти
    close(fd);
    return 0;
}
