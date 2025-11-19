#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main(void) {
    pid_t pid;
    int status;

    // Первая часть - параллельное выполнение
    printf("=== Первая часть ===\n");
    pid = fork();
    
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {
        // Дочерний процесс
        execlp("cat", "cat", "/var/log/syslog", NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    } else {
        // Родительский процесс
        printf("Родитель: дочерний процесс создан с PID %d\n", pid);
        printf("Родитель: продолжаю работу параллельно\n");
        sleep(1); // Даем время для демонстрации
    }
    
    // Вторая часть - ожидание завершения
    printf("\n=== Вторая часть ===\n");
    pid = fork();
    
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {
        // Дочерний процесс
        execlp("cat", "cat", "/var/log/syslog", NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    } else {
        // Родительский процесс ждет завершения
        printf("Родитель: ожидаю завершения дочернего процесса\n");
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("Родитель: дочерний процесс завершился со статусом %d\n", 
                   WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Родитель: дочерний процесс убит сигналом %d\n", 
                   WTERMSIG(status));
        }
        
        printf("Родитель: это последняя строка после завершения дочернего процесса\n");
    }
    
    return 0;
}