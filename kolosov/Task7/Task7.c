#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define MAX_LINES 1000

struct LineTable {
    off_t offset;
    size_t length;
};

volatile sig_atomic_t timeout_occurred = 0;

void timeout_handler(int sig) {
    timeout_occurred = 1;
}

void print_entire_file(char *file_map, size_t file_size) {
    printf("\n\nПолное содержимое файла:\n");
    fwrite(file_map, 1, file_size, stdout);
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

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("Ошибка получения информации о файле");
        close(fd);
        exit(1);
    }
    
    size_t file_size = sb.st_size;

    char *file_map = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_map == MAP_FAILED) {
        perror("Ошибка отображения файла");
        close(fd);
        exit(1);
    }

    struct LineTable table[MAX_LINES];
    int line_count = 0;

    table[0].offset = 0;
    
    for (off_t i = 0; i < file_size; i++) {
        if (file_map[i] == '\n') {
            table[line_count].length = i - table[line_count].offset;
            line_count++;
            if (line_count >= MAX_LINES) {
                fprintf(stderr, "Слишком много строк в файле\n");
                munmap(file_map, file_size);
                close(fd);
                exit(1);
            }
            table[line_count].offset = i + 1;
        }
    }

    if (line_count == 0 || table[line_count].offset < file_size) {
        table[line_count].length = file_size - table[line_count].offset;
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
            print_entire_file(file_map, file_size);
            munmap(file_map, file_size);
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
            printf("Строка %d: %.*s", line_num, (int)entry->length, 
                   file_map + entry->offset);
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

    munmap(file_map, file_size);
    close(fd);
    return 0;
}