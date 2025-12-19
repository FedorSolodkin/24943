#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int count = 0;

void sigint_handler(int signo){
    (void)signo;
    signal(SIGINT, sigint_handler);
    count++;
    write(1, "\a", 1);
}

void sigquit_handler(int signo){
    (void)signo;
    printf("Прозвучало %d сигналов\n", count);
    _exit(0);
}

int main(void) {
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigquit_handler);
    while (1) {
        pause();
    }
    return 0;
}