#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>

int opt;
struct rlimit rlimit_io;
char buffer[128];
extern char **environ;
char **envp;
int ret;

int main(int argc, char *argv[])
{
    while ((opt = getopt(argc, argv, "ispuU:cC:dvV:h")) != -1)
    {
        switch (opt)
        {
        case 'i':
            printf(
                "UID: %d, Effective UID: %d, GID: %d, Effective GID: %d\n",
                getuid(),
                geteuid(),
                getgid(),
                getegid());
            break;
        case 's':
            ret = setpgid(0, 0);
            if (ret) printf("%d\n", errno);
            break;
        case 'p':
            printf(
                "Process ID: %d, Parent PID: %d, Process Group ID: %d\n",
                getpid(),
                getppid(),
                getpgrp());
            break;
        case 'u':
            // That "rlimit" struct has many limits, so I print the one that shell command returns.
            getrlimit(RLIMIT_FSIZE, &rlimit_io);
            printf("%ld\n", rlimit_io.rlim_cur);
            break;
        case 'U':
            getrlimit(RLIMIT_FSIZE, &rlimit_io);
            rlimit_io.rlim_cur = atol(optarg);
            setrlimit(RLIMIT_FSIZE, &rlimit_io);
            break;
        case 'c':
            getrlimit(RLIMIT_CORE, &rlimit_io);
            printf("%ld\n", rlimit_io.rlim_cur);
            break;
        case 'C':
            getrlimit(RLIMIT_CORE, &rlimit_io);
            rlimit_io.rlim_cur = atol(optarg);
            setrlimit(RLIMIT_CORE, &rlimit_io);
            break;
        case 'd':
            printf("%s\n", getcwd(buffer, sizeof(buffer)));
            break;
        case 'v':
            envp = environ;
            while (*envp)
            {
                printf("%s\n", *envp);
                envp++;
            }
            break;
        case 'V':
            strcpy(buffer, optarg);
            char *name = strtok(buffer, "=");
            char *val = strtok(NULL, "=");
            setenv(buffer, val, 0);
            // printf("Set %s to %s\n", buffer, val);
            break;
        
        case 'h':
            // I've copied it from the tasks file
            printf(
                "-i  Печатает реальные и эффективные идентификаторы пользователя и группы.\n"
                "-s  Процесс становится лидером группы. Подсказка: смотри setpgid(2).\n"
                "-p  Печатает идентификаторы процесса, процесса-родителя и группы процессов.\n"
                "-u  Печатает значение ulimit\n"
                "-Unew_ulimit  Изменяет значение ulimit. Подсказка: смотри atol(3C) на странице руководства strtol(3C)\n"
                "-c  Печатает размер в байтах core-файла, который может быть создан.\n"
                "-Csize  Изменяет размер core-файла\n"
                "-d  Печатает текущую рабочую директорию\n"
                "-v  Распечатывает переменные среды и их значения\n"
                "-Vname=value  Вносит новую переменную в среду или изменяет значение существующей переменной\n"
            );
            break;

        default:
            break;
        }

    }
    
    return 0;
}
