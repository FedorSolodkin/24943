#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define MAX_LINES 1000
#define BUFFER_SIZE 1024

struct LineTable {
    off_t offset;
    size_t length;
};

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

    int line_num;
    while (1) {
        printf("Введите номер строки (0 для выхода): ");
        
        int result = scanf("%d", &line_num);
        if (result != 1) {
            printf("Ошибка: введите целое число\n");
            while (getchar() != '\n');
            continue;
        }

        if (line_num == 0) break;
        
        if (line_num < 1 || line_num > line_count) {
            printf("Неверный номер строки. Допустимый диапазон: 1-%d\n", line_count);
            continue;
        }

        struct LineTable *entry = &table[line_num - 1];
        lseek(fd, entry->offset, SEEK_SET);
        
        ssize_t bytes_to_read = entry->length;
        if (bytes_to_read >= BUFFER_SIZE) {
            bytes_to_read = BUFFER_SIZE - 1;
        }

        ssize_t n = read(fd, buffer, bytes_to_read);
        if (n > 0) {
            buffer[n] = '\0';
            printf("Строка %d: %s \n", line_num, buffer);
        }
    }

    close(fd);
    return 0;
}