#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


int main(){
    struct termios old_tio, new_tio;

    tcgetattr(0, &old_tio);
    new_tio = old_tio;
    new_tio.c_lflag &= ~(ICANON | ECHO);
    new_tio.c_cc[VMIN] = 1;
    new_tio.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_tio);

    char erase = new_tio.c_cc[VERASE];
    char kill = new_tio.c_cc[VKILL];

    char buffer[1000];
    int pos = 0;
    int col = 0;

    while(1){

        char c;
        if (read(0, &c, 1) != 1) break;
        if (c == 4 && pos == 0){
            break;
        }else if(c == '\n' || c == '\r'){
            write(1, "\n", 1);
            pos = 0;
            col = 0;
        }else if(c == erase){
            if (pos > 0){
                write(1, "\b \b", 3);
                pos--;
                col = (col > 0) ? col - 1 : 0;
            } else{
                write(1, "\7", 1);
            }
        } else if(c == kill){
            while (pos > 0){
                write(1, "\b \b", 3);
                pos--;
            }
            col = 0;
        }else if (c == 23){
            if (pos == 0){
                write(1, "\a", 1);
            } else{
                while (pos > 0 && buffer[pos-1] == ' ') {
                    write(1, "\b \b", 3);
                    pos--; 
                    col--;
                }

                while (pos > 0 && buffer[pos-1] != ' ') {
                    write(1, "\b \b", 3);
                    pos--; col--;
                }
                if (col < 0) col = 0;
            }
        }else if (c >= 32 && c < 127) {
            if (pos < 999) {
                buffer[pos++] = c;
                write(1, &c, 1);
                col++;

                if (col > 40) {

                    int i = pos - 1;
                    while (i > 0 && buffer[i-1] != ' ') i--;
                    int word_len = pos - i;

                    if (i > 0 && word_len <= 40) {
                        for (int k = 0; k < word_len; k++) {
                            write(1, "\b \b", 3);
                        }

                        write(1, "\n", 1);

                        memmove(buffer, buffer + i, word_len);
                        pos = word_len;
                        col = word_len;

                        write(1, buffer, word_len);
                    }
                }
            }
        }else{
            write(1, "\a", 1);
        }
    }
    tcsetattr(0, TCSANOW, &old_tio);
    write(1, "\n", 1);

    return 0;
}
