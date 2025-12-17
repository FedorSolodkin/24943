#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 40
#define BUFFER_SIZE 1024

void enable_raw_mode(struct termios *orig_termios) {
    tcgetattr(STDIN_FILENO, orig_termios);
    struct termios raw = *orig_termios;

    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disable_raw_mode(struct termios *orig_termios) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, orig_termios);
}

void beep() {
    write(STDOUT_FILENO, "\a", 1);
}

void delete_last_character(char *buffer, int *len, int *current_col) {
    if (*len > 0) {
        (*len)--;
        (*current_col)--;
        buffer[*len] = '\0';
        printf("\b \b");
        fflush(stdout);
    }
}

void delete_line(char *buffer, int *len, int *current_col) {
    while (*len > 0) {
        printf("\b \b");
        fflush(stdout);
        (*len)--;
        (*current_col)--;
    }
    buffer[0] = '\0';
    *current_col = 0;
}
void delete_last_word(char *buffer, int *len, int *current_col) {
    if (*len == 0) return;
    
    int i = *len - 1;
    
    // 1. Пропускаем пробелы в конце
    while (i >= 0 && isspace(buffer[i])) i--;
    
    // 2. Пропускаем слово
    while (i >= 0 && !isspace(buffer[i])) i--;
    
    // 3. Пропускаем пробелы перед словом
    while (i >= 0 && isspace(buffer[i])) i--;
    
    int new_len = i + 1;
    int deleted = *len - new_len;
    
    for (int j = 0; j < deleted; j++) {
        printf("\b \b");
    }
    fflush(stdout);
    
    *len = new_len;
    *current_col = *len;
    buffer[*len] = '\0';
}
// Перенос строки при достижении 40 символов
void handle_line_break(char *buffer, int *len, int *current_col) {
    if (*current_col >= MAX_LINE_LENGTH) {
        // Ищем последний пробел для переноса слова
        int i = *len - 1;
        while (i >= 0 && i > *len - 10) {
            if (isspace(buffer[i])) {
                printf("\n");
                *current_col = *len - i - 1;
                
                for (int j = i + 1; j < *len; j++) {
                    printf("%c", buffer[j]);
                }
                fflush(stdout);
                return;
            }
            i--;
        }
        
        // Если пробел не нашли
        printf("\n");
        *current_col = 0;
    }
}

void process_input() {
    char buffer[BUFFER_SIZE] = {0};
    int len = 0;
    int current_col = 0;
    char c;

    while (1) {
        if (read(STDIN_FILENO, &c, 1) <= 0) continue;

        // Ctrl-D
        if (c == 4) {
            if (current_col == 0) {
                printf("\n");
                break;
            } else continue;
        }

        // Erase 
        if (c == 8 || c == 127) {
            if (len > 0) {
                delete_last_character(buffer, &len, &current_col);
            } else beep();
            continue;
        }

        // KILL (Ctrl-U)
        if (c == 21) {
            delete_line(buffer, &len, &current_col);
            continue;
        }

        // Ctrl-W
        if (c == 23) {
            delete_last_word(buffer, &len, &current_col);
            continue;
        }

        // Enter
        if (c == '\n') {
            printf("\n");
            len = 0;
            current_col = 0;
            buffer[0] = '\0';
            continue;
        }

        // Печатаемые символы
        if (isprint(c)) {
            if (len < BUFFER_SIZE - 1) {
                buffer[len] = c;
                len++;
                buffer[len] = '\0';
                
                printf("%c", c);
                current_col++;
                
                handle_line_break(buffer, &len, &current_col);
                
                fflush(stdout);
            } else beep();
        } else beep();  
    }
}

int main() {
    struct termios orig_termios;

    enable_raw_mode(&orig_termios);
    process_input();
    disable_raw_mode(&orig_termios);

    return 0;
}