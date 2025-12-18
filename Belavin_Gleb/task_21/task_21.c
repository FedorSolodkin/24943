#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

int count = 0;

void beep(int sig) {
    count++;
    printf("\007");
    fflush(stdout);
    signal(SIGINT, beep);
}

void quit(int sig) {
    printf("\nSignal count: %d\n", count);
    exit(0);
}

int main() {
    signal(SIGINT, beep);
    signal(SIGQUIT, quit);
    
    printf("CTRL-C = beep, CTRL-\\ = quit\n");
    
    while(1)
        pause();
}
