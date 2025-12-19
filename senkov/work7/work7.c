#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <signal.h>

#define MAX_LINES 500
#define BUFSZ 256

int fd;  // Глобальная переменная для доступа из обработчика сигнала
int first_input = 1;  // Флаг первого ввода
char *file_data;      // Указатель на отображенный файл
off_t file_size;      // Размер файла

// Обработчик сигнала SIGALRM - печатает весь файл ИЗ ПАМЯТИ
void timeout_handler(int sig) {
    printf("\nTime's up! Printing entire file...\n");
    
    // Выводим весь файл ИЗ ПАМЯТИ (mmap)
    if (file_data != NULL && file_size > 0) {
        fwrite(file_data, 1, file_size, stdout);
    }
    
    // Освобождение ресурсов
    if (file_data != NULL) {
        munmap(file_data, file_size);
    }
    close(fd);
    exit(0);
}

int main(int argc, char **argv) {
    int line_count = 0;
    char *line_start[MAX_LINES];  // Указатели на начало строк
    int line_length[MAX_LINES];   // Длины строк
    
    char input[BUFSZ];
    int line_no;

    // Проверка аргументов
    if (argc < 2) { 
        fprintf(stderr, "Usage: %s <file>\n", argv[0]); 
        return 1; 
    }
    
    // Открытие файла
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) { 
        perror(argv[1]); 
        return 1; 
    }

    // Определение размера файла
    file_size = lseek(fd, 0, SEEK_END);
    if (file_size == (off_t)-1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    // Проверка на пустой файл
    if (file_size == 0) {
        printf("File is empty\n");
        close(fd);
        return 0;
    }

    // Отображение файла в память
    file_data = mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);
    if (file_data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    // Установка обработчика сигнала SIGALRM
    if (signal(SIGALRM, timeout_handler) == SIG_ERR) {
        perror("signal");
        munmap(file_data, file_size);
        close(fd);
        return 1;
    }

    // Построение таблицы строк
    line_start[0] = file_data;  // Первая строка начинается с начала файла
    line_length[0] = 0;
    line_count = 1;

    for (off_t i = 0; i < file_size && line_count < MAX_LINES; i++) {
        if (file_data[i] == '\n') {
            // Сохраняем длину текущей строки (включая \n)
            line_length[line_count - 1] = i - (line_start[line_count - 1] - file_data) + 1;
            
            // Начало следующей строки (если не конец файла)
            if (i + 1 < file_size && line_count < MAX_LINES) {
                line_start[line_count] = &file_data[i + 1];
                line_length[line_count] = 0;
                line_count++;
            }
        }
    }

    // Обработка последней строки (если файл не заканчивается на \n)
    if (line_count > 0 && line_length[line_count - 1] == 0) {
        line_length[line_count - 1] = file_size - (line_start[line_count - 1] - file_data);
    }

    printf("\n=== DEBUG: MMap Line Table ===\n");
    printf("File mapped at: %p, Size: %ld bytes\n", file_data, file_size);
    printf("Total lines: %d\n", line_count);
    printf("Line | Memory Addr  | Length | Content Preview\n");
    printf("-----+-------------+--------+----------------\n");
    
    for (int j = 0; j < line_count; j++) {
        // Создаем предпросмотр содержимого строки
        char preview[21];
        int preview_len = line_length[j] < 20 ? line_length[j] : 20;
        
        // Копируем данные напрямую из памяти (mmap)
        memcpy(preview, line_start[j], preview_len);
        preview[preview_len] = '\0';
        
        // Заменяем непечатаемые символы
        for (int k = 0; k < preview_len; k++) {
            if (preview[k] == '\n') preview[k] = '\\';
            if (preview[k] == '\t') preview[k] = '→';
            if (preview[k] < 32 || preview[k] > 126) preview[k] = '.';
        }
        
        // Выводим информацию о строке (адрес в памяти вместо смещения в файле)
        printf("%4d | %11p | %6d | %s\n", 
               j+1, line_start[j], line_length[j], preview);
    }
    
    printf("=== End of Table ===\n\n");
    
    // ВАЖНО: Сообщение о таймере для пользователя
    printf("You have 5 seconds for FIRST input. Enter 0 to exit.\n");

    // Интерактивный цикл
    for (;;) {
        printf("Line number: ");
        fflush(stdout);  // Сбросить буфер вывода
        
        // Установить таймер на 5 секунд ТОЛЬКО для первого ввода
        if (first_input) {
            alarm(5);
        }
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            alarm(0);  // Отключить таймер при выходе
            break;
        }
        
        // Проверка на переполнение буфера (если ввод не содержит \n)
        if (strchr(input, '\n') == NULL) {
            // Буфер переполнен, очищаем остаток
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            fprintf(stderr, "Error: Input too long. Maximum %zu characters allowed.\n", sizeof(input) - 1);
            continue;
        }
        
        // Отключить таймер после успешного ввода
        alarm(0);
        
        // Сбрасываем флаг первого ввода после успешного получения данных
        if (first_input && input[0] != '\0') {
            first_input = 0;
            printf("First input received. No more time limits.\n");
        }
        
        input[strcspn(input, "\n")] = '\0';
        
        if (input[0] == '\0') {
            continue;
        }
        
        char *endptr;
        line_no = (int)strtol(input, &endptr, 10);
        
        if (*endptr != '\0' && *endptr != '\n') {
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        if (line_no == 0) {
            break;
        }
        
        if (line_no < 1 || line_no > line_count) {
            fprintf(stderr, "Bad Line Number. Available lines: 1-%d\n", line_count);
            continue;
        }

        // Вывод строки используя отображение в память
        int idx = line_no - 1;
        fwrite(line_start[idx], 1, line_length[idx], stdout);
        
        // Если строка не заканчивается на \n, добавляем его
        if (line_length[idx] > 0 && line_start[idx][line_length[idx] - 1] != '\n') {
            printf("\n");
        }
    }

    // Отключить таймер перед нормальным выходом
    alarm(0);
    
    // Освобождение ресурсов
    munmap(file_data, file_size);
    close(fd);
    return 0;
}
