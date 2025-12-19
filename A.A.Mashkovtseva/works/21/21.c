#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>


volatile int counter = 0;
volatile int quit_flag = 0;
const char bell = '\a';

void handler(int sig) {
    if (sig == SIGQUIT) {
        quit_flag = 1;
    }
    else if (sig == SIGINT) {
        counter++;
        write(STDOUT_FILENO, &bell, 1);
    }
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    

    if (sigaction(SIGINT, &sa, NULL) == -1 || sigaction(SIGQUIT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    while (!quit_flag) pause();

    printf("BELL number: %d\n", counter);
    return 0;
}