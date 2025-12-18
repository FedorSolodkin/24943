#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#define MAX_LINE 40
#define CTRL_D 4
#define CTRL_G 7
#define CTRL_W 23

static struct termios orig_termios;
static char line[MAX_LINE + 1];
static int pos = 0;

void restore_terminal(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void setup_terminal(void) {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(restore_terminal);
    raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void beep(void) {
    write(STDOUT_FILENO, "\007", 1);
}

void erase_char(void) {
    write(STDOUT_FILENO, "\b \b", 3);
}

void erase_last(void) {
    if (pos > 0) {
        pos--;
        erase_char();
    }
}

void erase_line(void) {
    while (pos > 0) {
        pos--;
        erase_char();
    }
}

void erase_word(void) {
    while (pos > 0 && line[pos - 1] == ' ') {
        pos--;
        erase_char();
    }
    while (pos > 0 && line[pos - 1] != ' ') {
        pos--;
        erase_char();
    }
    while (pos > 0 && line[pos - 1] == ' ') {
        pos--;
        erase_char();
    }
}

void newline(void) {
    write(STDOUT_FILENO, "\r\n", 2);
    pos = 0;
}

void add_char(char c) {
    if (pos >= MAX_LINE) {
        int word_start = pos;
        while (word_start > 0 && line[word_start - 1] != ' ')
            word_start--;
        
        if (word_start > 0) {
            int word_len = pos - word_start;
            char word[MAX_LINE + 1];
            memcpy(word, line + word_start, word_len);
            
            while (pos > word_start) {
                pos--;
                erase_char();
            }
            newline();
            
            for (int i = 0; i < word_len; i++) {
                line[pos++] = word[i];
                write(STDOUT_FILENO, &word[i], 1);
            }
        } else {
            newline();
        }
    }
    
    line[pos++] = c;
    write(STDOUT_FILENO, &c, 1);
}

int main(void) {
    char c;
    char erase_ch, kill_ch;
    struct termios t;
    
    tcgetattr(STDIN_FILENO, &t);
    erase_ch = t.c_cc[VERASE];
    kill_ch = t.c_cc[VKILL];
    
    setup_terminal();
    
    while (read(STDIN_FILENO, &c, 1) == 1) {
        if (c == CTRL_D && pos == 0) {
            break;
        } else if (c == erase_ch) {
            erase_last();
        } else if (c == kill_ch) {
            erase_line();
        } else if (c == CTRL_W) {
            erase_word();
        } else if (c == '\n' || c == '\r') {
            newline();
        } else if (isprint((unsigned char)c)) {
            add_char(c);
        } else {
            beep();
        }
    }
    
    write(STDOUT_FILENO, "\r\n", 2);
    return 0;
}
