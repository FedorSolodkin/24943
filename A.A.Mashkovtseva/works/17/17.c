#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#define LINESIZE 40
#define BUFFER 512

void BACKSPACE (int fd) {
    write(fd, "\b \b", 3);
}

int main() {
    struct termios tty, savetty;
    int fd = open("/dev/tty", O_RDWR);
    tcgetattr(fd, &tty);
    savetty = tty;
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;
    tty.c_lflag &= ~(ISIG | ICANON | ECHO);
    tcsetattr(fd, TCSANOW, &tty);

    unsigned char eof = savetty.c_cc[VEOF];
    unsigned char erase = savetty.c_cc[VERASE];
    unsigned char kill = savetty.c_cc[VKILL];
    unsigned char worde = savetty.c_cc[VWERASE];
    char ch;
    char buffer[BUFFER];
    int pos = 0;
    int savepos = 0;
    int newpos = 0;
    while (read(fd, &ch, 1) == 1) {
        if (ch == eof && pos == 0) break; // Ctrl-D

        if (ch == erase && pos > 0) {
            BACKSPACE(fd);
            pos--;
        }
        else if (ch == kill) {
            while (pos > 0) {
                BACKSPACE(fd);
                pos--;
            }
        }
        else if (ch == worde) { // Ctrl-W
            while (pos > 0 && isspace(buffer[pos - 1])) {
                BACKSPACE(fd);
                pos--;
            }
            while (pos > 0 && !isspace(buffer[pos - 1])) {
                BACKSPACE(fd);
                pos--;
            }
        }
        else if (ch == '\n') {
            write(fd, &ch, 1);
            pos = 0;
        }
        else if (!isprint(ch)) {
            write(fd, "\a", 1); // bell
        }
        else {
            write(fd, &ch, 1);
            buffer[pos++] = ch;
        }

        if (pos >= LINESIZE && !isspace(ch)) {
            savepos = pos;
            newpos = 0;
            while (pos > 0 && !isspace(buffer[pos - 1])) pos--;
            if (pos > 0) {
                
                for (int i = pos; i < savepos; i++) {
                    BACKSPACE(fd);
                    buffer[newpos++] = buffer[i];
                }
                pos = newpos;
                write(fd, "\n", 1);
                for (int i = 0; i < pos; i++) write(fd, &buffer[i], 1);
            }
            else write(fd, "\n", 1);
        }
    }


    tcsetattr(fd, TCSANOW, &savetty);
    close(fd);
    return 0;
}