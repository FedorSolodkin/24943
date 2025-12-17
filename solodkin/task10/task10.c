#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int status;
    pid_t pid;

    
    if (argc == 1) {
        fprintf(stderr, "Использование: %s <команда> [аргументы...]\n", argv[0]);
        return 1;
    }

  
    pid = fork();
    if (pid == -1) {
        perror("Ошибка fork");
        return 1;
    }

    if (pid == 0) {
        
        
        execvp(argv[1], &argv[1]);
        
        perror(argv[1]);
        exit(127); 
    }


    if (wait(&status) == -1) {
        perror("Ошибка wait");
        return 1;
    }

  
    if (WIFEXITED(status)) {
        
        printf("Код завершения: %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        
        printf("Процесс завершен сигналом: %d\n", WTERMSIG(status));
    }

    return 0;
}