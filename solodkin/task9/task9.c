#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid;
    int status;

    
    pid = fork();
    
    if (pid == -1) {
        perror("Ошибка при создании процесса");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
    
        printf("Дочерний процесс: запускаю cat для файла long_file.txt\n");
        execlp("cat", "cat", "long_file.txt", NULL);
        
        
        perror("Ошибка при выполнении cat");
        exit(EXIT_FAILURE);
    } else {
      
        printf("Родительский процесс: работаю параллельно с cat\n");
        printf("PID дочернего процесса: %d\n", pid);
        printf("Родитель печатает этот текст, пока дочерний процесс выполняется\n");
        

        printf("Родитель ожидает завершения дочернего процесса...\n");
        if (waitpid(pid, &status, 0) == -1) {
            perror("Ошибка при ожидании процесса");
            exit(EXIT_FAILURE);
        }
        
   
        if (WIFEXITED(status)) {
            printf("Дочерний процесс завершился успешно со статусом %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Дочерний процесс завершен сигналом %d\n", WTERMSIG(status));
        }
        
   
        printf("=== ЭТА СТРОКА ВЫВЕДЕНА ПОСЛЕ ЗАВЕРШЕНИЯ ДОЧЕРНЕГО ПРОЦЕССА ===\n");
    }

    return 0;
}