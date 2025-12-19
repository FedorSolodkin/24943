#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <имя_файла>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Ошибка fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        execlp("cat", "cat", argv[1], NULL);
        
        perror("Ошибка exec");
        exit(EXIT_FAILURE);
    } else {
        printf("Родитель: процесс запущен. PID потомка: %d\n", pid);
        
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("Ошибка waitpid");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status)) {
            printf("Родитель: Потомок завершился с кодом %d. Финальное сообщение.\n", WEXITSTATUS(status));
        } else {
            printf("Родитель: Потомок завершился аномально.\n");
        }
    }

    return EXIT_SUCCESS;
}
