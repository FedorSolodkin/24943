#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#define MAX_LINE 40

void erase_character(int fd, int *pos) {
    if (*pos > 0) {
        write(fd, "\b \b", 3);
        (*pos)--;
    }
}

void erase_word(int fd, char *buffer, int *pos) {
    while (*pos > 0 && isspace(buffer[*pos - 1])) {
        erase_character(fd, pos);  
    }
    while (*pos > 0 && !isspace(buffer[*pos - 1])) {
        erase_character(fd, pos);  
    }
}

void check_line_wrap(int fd, char *buffer, int *pos, int *column) {
    if (*column < MAX_LINE) {  
        return; 
    }
    

    int wrap_pos = *pos - 1;
    int found_space = -1;
    

    while (wrap_pos >= 0) {
        if (isspace(buffer[wrap_pos])) {
            found_space = wrap_pos;
           
            if (found_space < *pos - 1) {
                break;
            }
        }
        wrap_pos--;
    }
    
    if (found_space != -1 && found_space < *pos - 1) {
        int new_line_start = found_space + 1; 
        int chars_to_move = *pos - new_line_start;
        
        write(fd, "\n", 1);
        
        for (int i = 0; i < chars_to_move; i++) {
            write(fd, &buffer[new_line_start + i], 1);
        }
        
        for (int i = 0; i < chars_to_move; i++) {
            buffer[i] = buffer[new_line_start + i];
        }
        
        *pos = chars_to_move;
        *column = chars_to_move;
    } else {
        write(fd, "\n", 1);
        *column = 1;
    }
}

int main() {
    struct termios oldt, newt;
    char buffer[512];
    int pos = 0;
    int column = 0;
    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN] = 1;
    newt.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    unsigned char key_erase = oldt.c_cc[VERASE];      
    unsigned char key_kill = oldt.c_cc[VKILL];      
    unsigned char key_eof = oldt.c_cc[VEOF];          
    unsigned char key_werase = oldt.c_cc[VWERASE];    
    
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1) {
        if (c == key_eof && pos == 0) {
            break;
        }
        else if (c == key_erase) {  
            if (pos > 0) {
                erase_character(STDOUT_FILENO, &pos);  
                column--;
            } else {
                write(STDOUT_FILENO, "\a", 1);
            }
        }
        else if (c == key_kill) {
            while (pos > 0) {
                erase_character(STDOUT_FILENO, &pos);
                column--;
            }
        }
        else if (c == key_werase) {
            if (pos > 0) {
                erase_word(STDOUT_FILENO, buffer, &pos);
                column = 0;
                for (int i = 0; i < pos; i++) {
                    column++;
                    if (buffer[i] == '\t') column += 7;
                }
            } else {
                write(STDOUT_FILENO, "\a", 1);
            }
        }
        else if (c == '\n' || c == '\r') {
            write(STDOUT_FILENO, &c, 1);
            pos = 0;
            column = 0;
        }
        else if (isprint(c)) {
            if (pos < sizeof(buffer) - 1) {
                write(STDOUT_FILENO, &c, 1);
                buffer[pos++] = c;
                column++;
                check_line_wrap(STDOUT_FILENO, buffer, &pos, &column);
            } else {
                write(STDOUT_FILENO, "\a", 1);
            }
        }
        else {
            write(STDOUT_FILENO, "\a", 1);
        }
    }
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return 0;
}