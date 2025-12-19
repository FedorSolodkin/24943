#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#define MAX_LINES 1000
#define BUFFER_SIZE 1024

struct LineTable {
    off_t offset;
    size_t length;
};

volatile sig_atomic_t timeout_occurred = 0;

void timeout_handler(int sig) {
    timeout_occurred = 1;
}

void print_entire_file(int fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    lseek(fd, 0, SEEK_SET);
    printf("\n\nПолное содержимое файла:\n");
    
    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
        write(STDOUT_FILENO, buffer, bytes_read);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <файл>\n", argv[0]);
        exit(1);
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Ошибка открытия файла");
        exit(1);
    }

    struct LineTable table[MAX_LINES];
    char buffer[BUFFER_SIZE];
    int line_count = 0;
    off_t current_offset = 0;
    ssize_t bytes_read;

    table[0].offset = 0;
    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n') {
                table[line_count].length = current_offset + i - table[line_count].offset;
                line_count++;
                if (line_count >= MAX_LINES) {
                    fprintf(stderr, "Слишком много строк в файле\n");
                    close(fd);
                    exit(1);
                }
                table[line_count].offset = current_offset + i + 1;
            }
        }
        current_offset += bytes_read;
    }

    if (line_count == 0 || table[line_count].offset < current_offset) {
        table[line_count].length = current_offset - table[line_count].offset;
        line_count++;
    }

    printf("Таблица строк:\n");
    for (int i = 0; i < line_count; i++) {
        printf("Строка %d: смещение=%ld, длина=%zu\n", 
               i+1, table[i].offset, table[i].length);
    }

    struct sigaction sa;
    sa.sa_handler = timeout_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, NULL);

    printf("\nУ вас есть 5 секунд чтобы ввести номер строки.\n");
    
    int line_num;
    int result;
    
    while (1) {
        printf("Введите номер строки (0 для выхода): ");
        fflush(stdout);
        
        alarm(5);
        result = scanf("%d", &line_num);
        alarm(0);
        
        if (timeout_occurred) {
            printf("\nВремя вышло!\n");
            print_entire_file(fd);
            close(fd);
            return 0;
        }
        
        if (result == 1) {
            break;
        } else {
            printf("Ошибка: введите целое число\n");
            while (getchar() != '\n');
        }
    }

    while (1) {
        if (line_num == 0) break;
        
        if (line_num < 1 || line_num > line_count) {
            printf("Неверный номер строки. Допустимый диапазон: 1-%d\n", line_count);
        } else {
            struct LineTable *entry = &table[line_num - 1];
            lseek(fd, entry->offset, SEEK_SET);
            
            ssize_t bytes_to_read = entry->length;
            if (bytes_to_read >= BUFFER_SIZE) {
                bytes_to_read = BUFFER_SIZE - 1;
            }

            ssize_t n = read(fd, buffer, bytes_to_read);
            if (n > 0) {
                buffer[n] = '\0';
                printf("Строка %d: %s", line_num, buffer);
            }
        }

        while (1) {
            printf("\nВведите номер строки (0 для выхода): ");
            result = scanf("%d", &line_num);
            
            if (result == 1) {
                break;
            } else {
                printf("Ошибка: введите целое число\n");
                while (getchar() != '\n');
            }
        }
    }

    close(fd);
    return 0;
}