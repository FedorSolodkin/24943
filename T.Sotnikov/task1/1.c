#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <limits.h>
#include <errno.h>

#define MAX_OPTIONS 100

struct rlimit r;

typedef struct 
{
    int option_char;
    char* argument;
} CommandOption;

int main(int argc, char** argv)
{
    CommandOption options[MAX_OPTIONS];
    int options_cnt = 0;

    int opt;
    opterr = 0;

    while ((opt = getopt(argc, argv, "ispuU:C:dcvV:")) != -1)
    {
        if (options_cnt >= MAX_OPTIONS)
        {
            fprintf(stderr, "Option limit (%d) exceeded. Further options will not be accepted.\n", MAX_OPTIONS);
            break;
        }

        options[options_cnt].option_char = opt;

        if (optarg != NULL)
            options[options_cnt].argument = strdup(optarg);
        else 
            options[options_cnt].argument = NULL;

        options_cnt++;
    }

    if (options_cnt == 0)
    {
        printf("No options were passed.\n");
        return 0;
    }

    for (int i = options_cnt - 1; i >= 0; i--)
    {
        char current_option = options[i].option_char;
        char* current_argument = options[i].argument;

        printf("Option execution: -%c\n", current_option);

        switch (current_option)
        {
            case 'i':
                printf("Real UID:      %d\n", getuid());
                printf("Effective UID: %d\n", geteuid());
                printf("Real GID:      %d\n", getgid());
                printf("Effective GID: %d\n", getegid());
                break;

            case 's':
            {
                pid_t pid = getpid();
                if (setpgid(pid, pid) == -1)
                    perror("setpgid");
                else
                    printf("Process became group leader.\n");
                break;
            }

            case 'p':
                printf("PID:  %d\n", getpid());
                printf("PPID: %d\n", getppid());
                printf("PGID: %d\n", getpgrp());
                break;

            case 'u':
            {
                getrlimit(RLIMIT_FSIZE, &r);
                printf("Ulimit size: %ld\n", (r.rlim_cur == RLIM_INFINITY) ? -1 : r.rlim_cur);
                break;
            }

            case 'U':
            {
                if (getrlimit(RLIMIT_FSIZE, &r) == -1)
                {
                    perror("getrlimit");
                    exit(1);
                }
                char* ptr = current_argument;
                r.rlim_cur = strtod(current_argument, &ptr);
                if (current_argument == ptr)
                {
                    perror("strtod");
                    exit(1);
                }
                if (setrlimit(RLIMIT_FSIZE, &r) == -1) 
                {
                    perror("setrlimit");
                    exit(1);
                }
                printf("Ulimit changed to: %lu\n", r.rlim_cur);
                break;
            }

            case 'c':
            {
                struct rlimit rl;
                getrlimit(RLIMIT_CORE, &rl);
                printf("the size in bytes of the core file that can be created: %lu\n", rl.rlim_cur);
                break;
            }

            case 'C':
            {
                struct rlimit rl;
                char* endptr;
                errno = 0;
                rl.rlim_cur = rl.rlim_max = strtoul(current_argument, &endptr, 10);
                if (errno != 0 || *endptr != '\0')
                {
                    fprintf(stderr, "Invalid numeric argument: %s\n", current_argument);
                    continue;
                }
                if (setrlimit(RLIMIT_CORE, &rl))
                    perror("setlimit");
                else
                    printf("The core file limit has been changed.\n");
                break;
            }

            case 'd':
            {
                char buf[PATH_MAX]; 
                if (!getcwd(buf, sizeof(buf)))
                    perror("getcwd");
                else
                    printf("Current working directory: %s\n", buf);
                break;
            }

            case 'v':
            {
                extern char** environ;
                printf("Current environment vars:\n");
                for (char** env = environ; *env != NULL; env++)
                    printf("%s\n", *env);
                break;
            }

            case 'V':
            {
                int idx = 0;
                for (; current_argument[idx] != '=' && current_argument[idx] != '\0'; idx++);
                char *eq = strchr(current_argument, '=');
                if (!eq || eq == current_argument) {
                    fprintf(stderr, "Argument must be in NAME=value format.\n");
                    break;
                }
                if (putenv(current_argument))
                    perror("putenv:");
                else
                    printf("Environment variable changed.\n");
                break;
            }

            case '?':
                fprintf(stderr, "Unknown option or missing argument.\n");
                break;

            default:
                break;
        }
        
        printf("\n");

        if (current_argument && current_option != 'V')
            free(current_argument);
    }

    return 0;
}
