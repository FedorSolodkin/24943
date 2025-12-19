#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <string.h>

#define MAX_BUF 1024
#define LINE_WIDTH 40

struct termios orig_termios;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        exit(1);
    }
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void redraw(char *buf, int len, int *prev_lines) {
    if (*prev_lines > 0) {
        printf("\033[%dA", *prev_lines);
    }
    printf("\r\033[J");

    int col = 0;
    int lines = 0;
    int i = 0;

    while (i < len) {
        int word_start = i;
        while (i < len && !isspace(buf[i])) i++;
        int word_len = i - word_start;

        int space_start = i;
        while (i < len && isspace(buf[i])) i++;
        int space_len = i - space_start;

        if (col + word_len > LINE_WIDTH && col > 0) {
            printf("\n");
            col = 0;
            lines++;
        }

        for (int k = word_start; k < word_start + word_len; k++) putchar(buf[k]);
        col += word_len;

        for (int k = space_start; k < space_start + space_len; k++) {
            if (col >= LINE_WIDTH) {
                printf("\n");
                col = 0;
                lines++;
            }
            putchar(buf[k]);
            col++;
        }
    }
    
    fflush(stdout);
    *prev_lines = lines; // Запоминаем высоту текста для следующей очистки
}

int main()
{
    char buf[MAX_BUF];
    int len = 0;
    int prev_lines = 0; // Сколько строк занял вывод в прошлый раз

    enableRawMode();

    while (1) {
        char c;
        if (read(STDIN_FILENO, &c, 1) == -1) break;

        if (c == '\004') { // CTRL-D
            if (len == 0) break; // Выход только если строка пуста
            else {
                printf("\a"); // Иначе сигнал
                fflush(stdout);
            }
        } 
        else if (c == 127 || c == '\b') { // ERASE (Backspace)
            if (len > 0) len--;
            else printf("\a");
        } 
        else if (c == '\025') { // KILL (Ctrl-U)
            len = 0;
        } 
        else if (c == '\027') { // CTRL-W
            // Удаляем пробелы в конце
            while (len > 0 && isspace(buf[len - 1])) len--;
            // Удаляем слово до следующего пробела
            while (len > 0 && !isspace(buf[len - 1])) len--;
        } 
        else if (isprint(c)) { // Печатаемый символ
            if (len < MAX_BUF - 1) {
                buf[len++] = c;
            } else {
                printf("\a"); // Буфер переполнен
            }
        } 
        else {
            printf("\a"); // BELL (Ctrl-G)
            fflush(stdout);
        }

        redraw(buf, len, &prev_lines);
    }

    return 0;
}