#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

int main() {
    struct termios old, new;
    tcgetattr(STDIN_FILENO, &old);
    new = old;
    new.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &new);

    char line[1024] = {0};
    int pos = 0;
    int c;

    while (1) {
        c = getchar();

        if (c == old.c_cc[VEOF] && pos == 0) break;

        if (c == old.c_cc[VKILL]) { // CTRL-U (KILL)
            pos = 0;
            printf("\033[2K\r");
            fflush(stdout);
            continue;
        }

        if (c == old.c_cc[VERASE]) { // BACKSPACE/DEL (ERASE)
            if (pos > 0) {
                pos--;
                printf("\b \b");
                fflush(stdout);
            }
            continue;
        }

        if (c == 0x17) { // CTRL-W (слово)
            while (pos > 0 && line[pos - 1] == ' ') {
                pos--;
                printf("\b \b");
            }
            while (pos > 0 && line[pos - 1] != ' ') {
                pos--;
                printf("\b \b");
            }
            fflush(stdout);
            continue;
        }

        if (c == '\n') {
            printf("\n");
            pos = 0;
            continue;
        }

        if (isprint(c)) {
            if (pos < 40) {
                line[pos++] = c;
                putchar(c);
                fflush(stdout);
            }
        } else {
            putchar('\a'); // BELL
            fflush(stdout);
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    return 0;
}
