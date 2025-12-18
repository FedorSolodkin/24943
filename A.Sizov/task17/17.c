#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#define MAX_LINE 40

int main() {
    struct termios oldt, newt;
    char line[MAX_LINE + 1];
    int pos = 0;

    if (tcgetattr(STDIN_FILENO, &oldt) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
    newt = oldt;

    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN] = 1;
    newt.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }

    printf("Введите текст (CTRL-D для выхода):\n");

    while (1) {
        char c;
        if (read(STDIN_FILENO, &c, 1) != 1) continue;

        // CTRL-D завершает программу, если курсор в начале строки
        if (c == 4 && pos == 0) {
            printf("\nВыход.\n");
            break;
        }

        // ERASE (Backspace)
        if (c == 127 || c == 8) {
            if (pos > 0) {
                pos--;
                printf("\b \b");
                fflush(stdout);
            } else {
                putchar('\a'); // CTRL-G сигнал
                fflush(stdout);
            }
            continue;
        }

        // KILL (Ctrl-U)
        if (c == 21) {
            while (pos > 0) {
                pos--;
                printf("\b \b");
            }
            fflush(stdout);
            continue;
        }

        // CTRL-W - удаляет последнее слово
        if (c == 23) {
            while (pos > 0 && isspace(line[pos - 1])) {
                pos--;
                printf("\b \b");
            }
            while (pos > 0 && !isspace(line[pos - 1])) {
                pos--;
                printf("\b \b");
            }
            fflush(stdout);
            continue;
        }

        if (!isprint(c) && !isspace(c)) {
            putchar('\a'); // CTRL-G
            fflush(stdout);
            continue;
        }

        if (pos >= MAX_LINE) {
            putchar('\n');
            pos = 0;
        }

        line[pos++] = c;
        putchar(c);
        fflush(stdout);
    }

    if (tcsetattr(STDIN_FILENO, TCSANOW, &oldt) == -1) {
        perror("tcsetattr restore");
        exit(EXIT_FAILURE);
    }

    return 0;
}
