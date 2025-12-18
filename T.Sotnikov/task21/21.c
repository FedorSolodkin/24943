#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t count = 0;

void handle_sigint(int sig) {
    count++;
    write(1, "\a", 1);
    write(1, "\nSIGNAL!!!\n\n", 11); // выводим надпись, т.к. сигнал не воспроизводится.
}

void handle_sigquit(int sig) {
    printf("\nПолучено сигналов SIGINT: %d\n", count);
    exit(0);
}

int main()
{
    // Ctrl+C
    if (signal(SIGINT, handle_sigint) == SIG_ERR) {
        perror("Ошибка при установке SIGINT");
        return 1;
    }

    // Ctrl+\ (Quit)
    if (signal(SIGQUIT, handle_sigquit) == SIG_ERR) {
        perror("Ошибка при установке SIGQUIT");
        return 1;
    }

    printf("Программа запущена. Жмите Ctrl+C для звука, Ctrl+\\ для выхода.\n");

    // Бесконечный цикл ожидания
    while (1) {
        pause(); // ждем любой сигнал
    }

    return 0;
}