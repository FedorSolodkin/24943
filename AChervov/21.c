#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>

const char ctrl_g = 0x07;

struct termios term;
struct termios term_bak;
char buf[16];
volatile sig_atomic_t bell_cnt;
volatile sig_atomic_t stop;

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
    tcsetattr(fd, TCSAFLUSH, &term);
}

void signal_handler(int sig)
{
    if (sig == SIGINT)
    {
        write(STDOUT_FILENO, &ctrl_g, 1);
        bell_cnt++;
    }
    else if (sig == SIGQUIT)
    {
        stop = 1;
    }
}

int main(int argc, char const *argv[])
{
    init_term(0);
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);

    bell_cnt = 0;
    stop = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);

    while (!stop)
    {
        pause();
    }

    printf("Bell has been triggered %d times.\n", bell_cnt);

    return 0;
}
