#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <ctype.h>

// Константы для читаемости
#define BUF_SIZE 41  // 40 символов + 1 для null-терминатора
#define TERM_WIDTH 40

// Коды клавиш
#define KEY_BACKSPACE 0x7F
#define KEY_KILL_LINE 0x15
#define KEY_WORD_ERASE 0x17
#define KEY_EOF 0x04
#define KEY_BELL 0x07

struct termios saved_tios;

// Восстановление настроек терминала
void restore_console() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_tios);
}

// Включение raw-режима
void setup_console() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &saved_tios);
    atexit(restore_console); // Авто-сброс при выходе

    raw = saved_tios;
    raw.c_lflag &= ~(ECHO | ICANON); // Без эхо и буферизации
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
    char buffer[BUF_SIZE]; // Буфер строки
    int pos = 0;           // Текущая позиция курсора
    char input;

    setup_console();

    // Основной цикл чтения
    while (read(STDIN_FILENO, &input, 1) == 1) {
        
        // --- Блок обработки управляющих символов ---
        
        // 1. ERASE (Backspace)
        if (input == KEY_BACKSPACE) {
            if (pos > 0) {
                pos--;
                buffer[pos] = '\0';
                // ANSI escape: Курсор влево + очистить до конца строки
                write(STDOUT_FILENO, "\033[D\033[K", 6);
            } else {
                putchar(KEY_BELL);
            }
        }
        // 2. KILL (Удалить строку)
        else if (input == KEY_KILL_LINE) {
            pos = 0;
            buffer[0] = '\0';
            // ANSI escape: Очистить всю строку + возврат каретки
            write(STDOUT_FILENO, "\033[2K\r", 5);
        }
        // 3. CTRL-W (Удалить слово)
        else if (input == KEY_WORD_ERASE) {
            if (pos == 0) {
                putchar(KEY_BELL);
                continue;
            }

            int old_pos = pos;
            
            // Удаляем пробелы в конце (если курсор сразу после слова)
            while (pos > 0 && buffer[pos - 1] == ' ') {
                pos--;
            }
            // Удаляем само слово до пробела
            while (pos > 0 && buffer[pos - 1] != ' ') {
                pos--;
            }
            
            buffer[pos] = '\0';
            
            // Вычисляем, на сколько сдвинуть курсор влево
            int diff = old_pos - pos;
            // Формируем ANSI команду динамически
            char seq[20];
            int n = sprintf(seq, "\033[%dD\033[K", diff);
            write(STDOUT_FILENO, seq, n);
        }
        // 4. CTRL-D (EOF)
        else if (input == KEY_EOF) {
            if (pos == 0) exit(0); // Выход только из начала строки
            else putchar(KEY_BELL);
        }
        // 5. Печатаемые символы
        else if (isprint(input)) {
            // Если буфер полон и мы не переносим (редкий кейс), просто newline
            if (pos >= TERM_WIDTH) {
                putchar('\n');
                pos = 0;
            }

            // Добавляем символ
            buffer[pos++] = input;
            buffer[pos] = '\0';
            putchar(input);

            // --- Логика переноса слов (Word Wrap) ---
            if (pos == TERM_WIDTH && input != ' ') {
                // Ищем начало слова, которое "наехало" на границу
                int w_start = pos - 1;
                while (w_start >= 0 && buffer[w_start] != ' ') {
                    w_start--;
                }
                w_start++; // Коррекция индекса

                // Если слово найдено и оно не занимает всю строку целиком
                if (w_start > 0 && w_start < TERM_WIDTH) {
                    int w_len = pos - w_start;

                    // 1. Визуально убираем слово с текущей строки
                    // (Курсор влево на N + очистка)
                    char seq[20];
                    sprintf(seq, "\033[%dD\033[K", w_len);
                    write(STDOUT_FILENO, seq, strlen(seq));

                    // 2. Перенос строки
                    putchar('\n');

                    // 3. Восстанавливаем слово на новой строке
                    // Сдвигаем память в буфере
                    memmove(buffer, &buffer[w_start], w_len);
                    pos = w_len;
                    buffer[pos] = '\0';

                    // Печатаем слово заново
                    printf("%s", buffer);
                }
            }
            fflush(stdout);
        }
        // 6. Все остальные символы
        else {
            putchar(KEY_BELL);
        }
        
        fflush(stdout);
    }

    return 0;
}
