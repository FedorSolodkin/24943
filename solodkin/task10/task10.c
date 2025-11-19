#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    pid_t pid;
    int status;
    
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <команда> [аргументы...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    pid = fork();
    
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {
        // Дочерний процесс
        // Создаем массив аргументов для execvp
        char **cmd_args = malloc((argc) * sizeof(char*));
        if (!cmd_args) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        
        // Первый аргумент - имя команды, остальные - аргументы команды
        for (int i = 1; i < argc; i++) {
            cmd_args[i-1] = argv[i];
        }
        cmd_args[argc-1] = NULL;
        
        // Выполняем команду
        execvp(cmd_args[0], cmd_args);
        
        // Если дошли сюда, значит execvp не удался
        perror("execvp");
        free(cmd_args);
        exit(EXIT_FAILURE);
    } else {
        // Родительский процесс
        printf("Запущена команда: %s\n", argv[1]);
        printf("PID дочернего процесса: %d\n", pid);
        
        // Ожидаем завершения дочернего процесса
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
        
        // Анализируем код завершения
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("Команда '%s' завершилась с кодом: %d\n", argv[1], exit_code);
            
            // Дополнительная информация о статусе
            if (exit_code == 0) {
                printf("Статус: УСПЕШНО\n");
            } else {
                printf("Статус: ОШИБКА\n");
            }
        } else if (WIFSIGNALED(status)) {
            printf("Команда '%s' завершена сигналом: %d\n", argv[1], WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            printf("Команда '%s' остановлена сигналом: %d\n", argv[1], WSTOPSIG(status));
        } else {
            printf("Неизвестный статус завершения команды '%s'\n", argv[1]);
        }
    }
    
    return 0;
}