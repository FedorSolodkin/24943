#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int signal_count = 0;

void sigint_handler(int sig) {
    printf("\a");
    fflush(stdout);
    signal_count++;
}

void sigquit_handler(int sig) {
    printf("\nПрограмма завершена. Звуковой сигнал прозвучал %d раз(а).\n", signal_count);
    exit(0);
}

int main() {
    signal(SIGINT, sigint_handler);
    signal(SIGQUIT, sigquit_handler);

    while (1) {
        pause();
        signal(SIGINT, sigint_handler);
        signal(SIGQUIT, sigquit_handler);
    }

    return 0;
}
