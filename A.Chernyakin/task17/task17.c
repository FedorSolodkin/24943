#include <termios.h>
#include <unistd.h>
#include <stdio.h>

int main(){
    struct termios oldt, raw;
    tcgetattr(0, &oldt);
    raw = oldt;

    raw.c_lflag &= ~(ICANON | ECHO | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(0, TCSANOW, &raw);

    char erase = raw.c_cc[VERASE];
    char killc = raw.c_cc[VKILL];

    char buf[80];
    int len = 0; 
    int col = 0;  

    char c;
    while (read(0, &c, 1) == 1) {
        if (c == 4) {
            if (len == 0) break;
            write(1, "\7", 1); 
        } else if (c == '\n' || c == '\r') {
            write(1, "\n", 1);
            len = 0;
            col = 0;
        } else if (c == erase) {
            if (len > 0) {
                write(1, "\b \b", 3);
                len--;
                col--;
            } else {
                write(1, "\7", 1);
            }
        } else if (c == killc) { 
            while (len > 0) {
                write(1, "\b \b", 3);
                len--;
            }
            col = 0;
        } else if (c == 23) { 
            if (len == 0) {
                write(1, "\7", 1);
            } else {
                while (len > 0 && buf[len-1] == ' ') {
                    write(1, "\b \b", 3);
                    len--; col--;
                }
                while (len > 0 && buf[len-1] != ' ') {
                    write(1, "\b \b", 3);
                    len--; col--;
                }
            }    
        }
        else if (c == 27) {
            write(1, "\7", 1); 
            char x;
            while (read(0, &x, 1) == 1) {
                if ((x >= 'A' && x <= 'Z') || x == '~')
            break;
            }
        }
        else if (c >= 32 && c < 127) {
            if (len < 79) {
                buf[len++] = c;
                write(1, &c, 1);
                col++;

                if (col > 40) {
                    int i = len - 1;
                    while (i > 0 && buf[i-1] != ' ') i--;
                    int wl = len - i;

                    if (i > 0 && wl <= 40) {
                        int k;
                        for (k = 0; k < wl; k++)
                            write(1, "\b \b", 3);

                        write(1, "\n", 1);
                        for (k = 0; k < wl; k++)
                            buf[k] = buf[i + k];

                        len = wl;
                        col = wl;
                        write(1, buf, wl);
                    }
                }
            }
        } else {
            write(1, "\7", 1);
        }
    }

    tcsetattr(0, TCSANOW, &oldt);
    return 0;
}
