#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

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

    line_info_t lines[MAX_LINES];
    int   line_count   = 0;
    off_t current_off  = 0;
    off_t line_start   = 0;
    char  buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // таблица строк
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n') {
                if (line_count < MAX_LINES) {
                    lines[line_count].offset = line_start;
                    lines[line_count].length = current_off + i - line_start;
                    line_count++;
                }
                line_start = current_off + i + 1;
            }
        }
        current_off += bytes_read;
    }

    if (bytes_read == -1) {
        perror("read");
        close(fd);
        return 1;
    }

    if (line_start < current_off && line_count < MAX_LINES) {
        lines[line_count].offset = line_start;
        lines[line_count].length = current_off - line_start;
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
        close(fd);
        return 1;
    }
    int input_received = 0;
    
    while (1) {
        char input[64];
        int  line_number;

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
            alarm(0);  // Отменяем таймер

            if (timeout_occurred) {
                print_entire_file(filename_global);
                break;
            }
            
            input_received = 1; 
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

        if (lseek(fd, line->offset, SEEK_SET) == (off_t)-1) {
            perror("lseek");
            continue;
        }

        char   line_buffer[BUFFER_SIZE];
        size_t read_length = line->length;
        if (read_length >= sizeof(line_buffer))
            read_length = sizeof(line_buffer) - 1;

        ssize_t actual_read = read(fd, line_buffer, read_length);
        if (actual_read == -1) {
            perror("read");
            continue;
        }

        line_buffer[actual_read] = '\0';
        printf("Line %d: %s\n", line_number, line_buffer);
    }

    alarm(0);
    close(fd);
    return 0;
}