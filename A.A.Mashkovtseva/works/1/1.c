#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/resource.h>
//#include <linux/limits.h>
#include <string.h>

extern char** environ;

typedef struct {
    char opt;
    char *arg;
} optrec;

int main(int argc, char *argv[])
{
    char options[] = "ispuU:cC:dvV:";
    long param;
    struct rlimit rl;
    optrec optarr[128];
    int optcount = 0;
    int i = 1;
    int double_dash = 0;

    printf("argc equals %d\n", argc);

    // Сначала собираем все опции и их аргументы
    while (i < argc) {
        if (!double_dash && argv[i][0] == '-' && argv[i][1] != '\0') {
            if (argv[i][1] == '-' && argv[i][2] == '\0') {
                double_dash = 1;
                i++;
                continue;
            }
            for (int j = 1; argv[i][j] != '\0'; ++j) {
                char c = argv[i][j];
                char *arg = NULL;
                // Опции с аргументом
                if ((c == 'U' || c == 'C' || c == 'V') && argv[i][j+1] == '\0' && i+1 < argc && (argv[i+1][0] != '-' || (argv[i+1][1] && (argv[i+1][1] < 'A' || (argv[i+1][1] > 'Z' && argv[i+1][1] < 'a') || argv[i+1][1] > 'z' || argv[i+1][2])))) {
                    arg = argv[i+1];
                    i++;
                }
                optarr[optcount].opt = c;
                optarr[optcount].arg = arg;
                optcount++;
            }
        } else {
            // не-опция или после '--'
            optarr[optcount].opt = 0;
            optarr[optcount].arg = argv[i];
            optcount++;
        }
        i++;
    }

    // Теперь обрабатываем опции в обратном порядке
    for (int k = optcount - 1; k >= 0; --k) {
        char c = optarr[k].opt;
        char *optarg = optarr[k].arg;
        switch (c) {
            case 0:
                printf("next parameter = %s\n", optarg);
                break;
            case 'i':
                printf("Real uid: %d\n", getuid());
                printf("Effective uid: %d\n", geteuid());
                printf("Real gid: %d\n", getgid());
                printf("Effective gid: %d\n", getegid());
                break;
            case 's':
                setpgid(0, 0);
                break;
            case 'p':
                printf("Process ID: %d\n", getpid());
                printf("Parent Process ID: %d\n", getppid());
                printf("Process Group ID: %d\n", getpgid(0));
                break;
            case 'u':
                getrlimit(RLIMIT_FSIZE, &rl);
                printf("ulimit: %ld\n", rl.rlim_cur);
                break;
            case 'U':
                getrlimit(RLIMIT_FSIZE, &rl);
                param = optarg ? atol(optarg) : 0;
                if (param < -1) {
                    printf("(U): You should use positive number or -1 for unlimited\n");
                    break;
                }
                rl.rlim_cur = param;
                setrlimit(RLIMIT_FSIZE, &rl);
                break;
            case 'c':
                getrlimit(RLIMIT_CORE, &rl);
                printf("Core file size limit: %ld\n", rl.rlim_cur);
                break;
            case 'C':
                getrlimit(RLIMIT_CORE, &rl);
                param = optarg ? atol(optarg) : 0;
                if (param < -1) {
                    printf("(C): You should use positive number or -1 for unlimited\n");
                    break;
                }
                rl.rlim_cur = param;
                setrlimit(RLIMIT_CORE, &rl);
                break;
            case 'd': {
                char cwd[500];
                getcwd(cwd, sizeof(cwd));
                printf("Current working directory: %s\n", cwd);
                break;
            }
            case 'v': {
                char **env;
                for (env = environ; *env != 0; env++) {
                    printf("%s\n", *env);
                }
                break;
            }
            case 'V': {
                char* checker = optarg ? strchr(optarg, '=') : NULL;
                if (optarg && checker != NULL && checker != optarg && checker[1] != '\0')
                    putenv(optarg);
                else
                    printf("You shold use \"NAME=valeue\" format for -V option\n");
                break;
            }
            case '?':
                break;
        }
    }
    return 0;
}