#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#define MAX_LINE 40

void erase_character(int fd, char *buffer, int *pos, int *column) {
    if (*pos > 0) {
        if (*column > 0) {
            write(fd, "\b \b", 3);
            (*pos)--;
            (*column)--;
        } else {
            int line_start = *pos;
            while (line_start > 0 && buffer[line_start - 1] != '\n') {
                line_start--;
            }
            
            if (line_start > 0) {
                write(fd, "\b \b", 3);
                (*pos)--;
                
                int prev_line_start = line_start - 1;
                while (prev_line_start > 0 && buffer[prev_line_start - 1] != '\n') {
                    prev_line_start--;
                }
                
                int prev_line_end = line_start - 1;
                int prev_line_length = prev_line_end - prev_line_start;
                
                *column = prev_line_length;
                
                write(fd, "\r", 1);
                for (int i = prev_line_start; i < prev_line_end; i++) {
                    write(fd, &buffer[i], 1);
                }
                for (int i = 0; i < prev_line_length; i++) {
                    write(fd, "\b", 1);
                }
            }
        }
    }
}

void erase_word(int fd, char *buffer, int *pos, int *column) {
    while (*pos > 0 && isspace(buffer[*pos - 1])) {
        erase_character(fd, buffer, pos, column);
    }
    while (*pos > 0 && !isspace(buffer[*pos - 1])) {
        erase_character(fd, buffer, pos, column);
    }
}

void check_line_wrap(int fd, char *buffer, int *pos, int *column) {
    if (*column < MAX_LINE) {
        return;
    }
    
    int wrap_pos = *pos - 1;
    int found_space = -1;
    
    while (wrap_pos >= 0 && (*pos - wrap_pos) <= MAX_LINE) {
        if (isspace(buffer[wrap_pos])) {
            found_space = wrap_pos;
            break;
        }
        wrap_pos--;
    }
    
    if (found_space != -1) {
        int new_line_start = found_space + 1;
        int chars_to_move = *pos - new_line_start;
        
        if (chars_to_move > 0) {
            for (int i = 0; i < chars_to_move + 1; i++) {
                write(fd, "\b \b", 3);
            }
            
            write(fd, "\n", 1);
            
            for (int i = 0; i < chars_to_move; i++) {
                write(fd, &buffer[new_line_start + i], 1);
            }
            
            for (int i = 0; i < chars_to_move; i++) {
                buffer[i] = buffer[new_line_start + i];
            }
            
            *pos = chars_to_move;
            *column = chars_to_move;
        }
    } else {
        write(fd, "\n", 1);
        *column = 0;
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
                erase_character(STDOUT_FILENO, buffer, &pos, &column);
            } else {
                write(STDOUT_FILENO, "\a", 1);
            }
        }
        else if (c == key_kill) {
            while (pos > 0) {
                erase_character(STDOUT_FILENO, buffer, &pos, &column);
            }
        }
        else if (c == key_werase) {
            if (pos > 0) {
                erase_word(STDOUT_FILENO, buffer, &pos, &column);
            } else {
                write(STDOUT_FILENO, "\a", 1);
            }
        }
        else if (c == '\n' || c == '\r') {
            write(STDOUT_FILENO, &c, 1);
            buffer[pos++] = '\n';
            buffer[pos] = '\0';
            printf("\nYou entered: %s", buffer);
            pos = 0;
            column = 0;
        }
        else if (isprint(c)) {
            if (pos < sizeof(buffer) - 1) {
                write(STDOUT_FILENO, &c, 1);
                buffer[pos++] = c;
                column++;
                
                if (isspace(c) || column >= MAX_LINE) {
                    check_line_wrap(STDOUT_FILENO, buffer, &pos, &column);
                }
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