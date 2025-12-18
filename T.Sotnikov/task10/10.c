#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <команда> [аргументы...]\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Ошибка fork");
        return 1;
    }

    if (pid == 0) {
        execvp(argv[1], &argv[1]);

        perror("Ошибка запуска");
        exit(127);
    } else {
        int status;
        
        wait(&status);

        if (WIFEXITED(status)) {
            printf("Код завершения: %d\n", WEXITSTATUS(status));
        } else {
            printf("Процесс был прерван или завершился с ошибкой.\n");
        }
    }

    return 0;
}