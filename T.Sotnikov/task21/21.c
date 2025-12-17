#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

// Глобальный счетчик, атомарный (безопасен для сигналов)
volatile sig_atomic_t count = 0;

// Обработчик для SIGINT (Ctrl+C)
void handle_sigint(int sig) {
    count++;                // Увеличиваем счетчик
    write(1, "\a", 1);      // Издаем звук (пишем bell-символ в stdout)
    write(1, "\nSIGNAL!!!\n\n", 11);
}

// Обработчик для SIGQUIT (Ctrl+\)
void handle_sigquit(int sig) {
    // Выводим результат и выходим
    printf("\nПолучено сигналов SIGINT: %d\n", count);
    exit(0);
}

int main() {
    // Регистрируем обработчик Ctrl+C
    if (signal(SIGINT, handle_sigint) == SIG_ERR) {
        perror("Ошибка при установке SIGINT");
        return 1;
    }

    // Регистрируем обработчик Ctrl+\ (Quit)
    if (signal(SIGQUIT, handle_sigquit) == SIG_ERR) {
        perror("Ошибка при установке SIGQUIT");
        return 1;
    }

    printf("Программа запущена. Жмите Ctrl+C для звука, Ctrl+\\ для выхода.\n");

    // Бесконечный цикл ожидания
    while (1) {
        pause(); // Ждем любой сигнал, чтобы не грузить процессор
    }

    return 0;
}