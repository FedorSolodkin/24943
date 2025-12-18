#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <limits.h>

extern char **environ;

typedef struct {
    char opt;
    char *arg;
} Opt;

void process_option(char opt, char *arg) {
    struct rlimit rl;
    char cwd[PATH_MAX];

    switch(opt) {
        case 'i':
            printf("UID real=%d effective=%d\n", getuid(), geteuid());
            printf("GID real=%d effective=%d\n", getgid(), getegid());
            break;
        case 's':
            setpgid(0, 0);
            break;
        case 'p':
            printf("PID=%d PPID=%d PGID=%d\n", getpid(), getppid(), getpgid(0));
            break;
        case 'u':
            getrlimit(RLIMIT_NOFILE, &rl);
            printf("ulimit=%ld\n", rl.rlim_cur);
            break;
        case 'U':
            rl.rlim_cur = atol(arg);
            setrlimit(RLIMIT_NOFILE, &rl);
            break;
        case 'c':
            getrlimit(RLIMIT_CORE, &rl);
            printf("core file size=%ld\n", rl.rlim_cur);
            break;
        case 'C':
            rl.rlim_cur = atol(arg);
            setrlimit(RLIMIT_CORE, &rl);
            break;
        case 'd':
            getcwd(cwd, sizeof(cwd));
            printf("cwd=%s\n", cwd);
            break;
        case 'v':
            for (char **e = environ; *e; e++)
                printf("%s\n", *e);
            break;
        case 'V':
            putenv(arg);
            break;
    }
}

int main(int argc, char *argv[]) {
    Opt opts[argc];
    int count = 0;
    int opt;
    opterr = 0;

    while ((opt = getopt(argc, argv, "ispuU:cCdvV:")) != -1) {
        opts[count].opt = opt;
        opts[count].arg = optarg;
        count++;
    }

    for (int i = count - 1; i >= 0; i--) {
        process_option(opts[i].opt, opts[i].arg);
    }

    return 0;
}
