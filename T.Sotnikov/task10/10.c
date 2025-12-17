#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    // Проверка: передана ли хотя бы одна команда
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <команда> [аргументы...]\n", argv[0]);
        return 1;
    }

    // Раздваиваем процесс
    pid_t pid = fork();

    if (pid < 0) {
        perror("Ошибка fork"); // Не удалось создать процесс
        return 1;
    }

    if (pid == 0) {
        // --- Дочерний процесс ---
        // Заменяем текущий процесс новой командой.
        // argv[1] — имя команды, &argv[1] — список аргументов (включая имя)
        execvp(argv[1], &argv[1]);

        // Если мы попали сюда, значит execvp не сработал (например, команды нет)
        perror("Ошибка запуска");
        exit(127); // Возвращаем ошибку (обычно 127, если command not found)
    } else {
        // --- Родительский процесс ---
        int status;
        
        // Ждем, пока дочерний процесс (pid) завершится
        wait(&status);

        // Проверяем, завершился ли процесс нормально
        if (WIFEXITED(status)) {
            printf("Код завершения: %d\n", WEXITSTATUS(status));
        } else {
            printf("Процесс был прерван или завершился с ошибкой.\n");
        }
    }

    return 0;
}