#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// I left it for the case if I'd have to make multi-line edit.
// struct line
// {
//     char buffer[41];
//     char len;
// }

const char ctrl_d = 0x04;
const char ctrl_g = 0x07;
const char ctrl_w = 0x17;
const char endl = '\n';
const char *backsp = "\b \b";

struct termios term;
struct termios term_bak;

char buf[41];
int pos = 0;
char stack[40];
int s_pos = 0;
int last_wd_len = 0;

void restore_term()
{
    tcsetattr(0, TCSAFLUSH, &term_bak);
}

void init_term(int fd)
{
    tcgetattr(fd, &term);
    term_bak = term;
    atexit(restore_term);
    term.c_lflag &= ~(ECHO|ICANON);
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;
    tcsetattr(fd, TCSAFLUSH, &term);
}

void write_printable(char c)
{
    if (pos > 39)
    {
        if (last_wd_len < 40)
        {
            while (pos > 0 && buf[pos - 1] != ' ')
            {
                stack[s_pos++] = buf[--pos];
                write(STDOUT_FILENO, backsp, 3);
            }
            write(STDOUT_FILENO, &endl, 1);
            pos = 0;
            while (s_pos > 0)
            {
                write(STDOUT_FILENO, &stack[s_pos - 1], 1);
                buf[pos++] = stack[--s_pos];
            }
            buf[pos++] = c;
            buf[pos] = '\0';
            last_wd_len++;
            write(STDOUT_FILENO, &c, 1);
        }
        else
        {
            write(STDOUT_FILENO, &ctrl_g, 1);
        }
    }
    else
    {
        buf[pos++] = c;
        buf[pos] = '\0';
        if (c == ' ')
        {
            last_wd_len = 0;
        }
        else
        {
            last_wd_len++;
        }
        write(STDOUT_FILENO, &c, 1);
    }
}

void recalc_wd_len()
{
    last_wd_len = 0;
    int temp_pos = pos;
    while (temp_pos > 0 && buf[temp_pos - 1] != ' ')
    {
        last_wd_len++;
        temp_pos--;
    }
}

int main(int argc, char const *argv[])
{
    init_term(STDIN_FILENO);
    
    while (1)
    {
        char rc;
        read(STDIN_FILENO, &rc, 1);

        if (pos == 0 && rc == ctrl_d) break;
        else if (rc == term.c_cc[VERASE])
        {
            if (pos > 0)
            {
                buf[--pos] = '\0';
                recalc_wd_len();
                write(STDOUT_FILENO, backsp, 3);
            }
        }
        else if (rc == term.c_cc[VKILL])
        {
            while (pos > 0)
            {
                buf[--pos] = '\0';
                write(STDOUT_FILENO, backsp, 3);
            }
            last_wd_len = 0;
        }
        else if (rc == ctrl_w)
        {
            while (pos > 0 && buf[pos-1] == ' ')
            {
                buf[--pos] = '\0';
                write(STDOUT_FILENO, backsp, 3);
            }
            while (pos > 0 && buf[pos-1] != ' ')
            {
                buf[--pos] = '\0';
                write(STDOUT_FILENO, backsp, 3);
            }
            last_wd_len = 0;
        }
        else if (rc == '\033')
        {
            write(STDOUT_FILENO, &ctrl_g, 1);
            read(STDIN_FILENO, &rc, 1);
            if (rc == '[')
            {
                read(STDIN_FILENO, &rc, 1);
                if (rc >= '0' && rc <= '9')
                {
                    read(STDIN_FILENO, &rc, 1);
                }
            }
            else
            {
                write_printable(rc);
            }
        }
        else if (rc == '\n')
        {
            write(STDOUT_FILENO, &endl, 1);
            last_wd_len = 0;
            pos = 0;
        }
        else if (isprint(rc))
        {
            write_printable(rc);
        }
        else
        {
            write(STDOUT_FILENO, &ctrl_g, 1);
        }
    }
    
    return 0;
}