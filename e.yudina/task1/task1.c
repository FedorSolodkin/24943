#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>

int main(int argc, char* argv[]) {
    const char* options = "ispudU:cC:V:";
    int c;

    while ((c = getopt(argc, argv, options)) != -1) {
        switch (c) {
        case 'i':
            printf("uid=%d euid=%d gid=%d egid=%d\n",
                getuid(), geteuid(), getgid(), getegid());
            break;

        case 's':
            if (setpgid(0, 0) == 0) {
                printf("pgid=%d\n", (int)getpgid(0));
            }
            else {
                perror("setpgid");
            }
            break;

        case 'p':
            printf("pid=%d ppid=%d pgid=%d\n",
                (int)getpid(), (int)getppid(), (int)getpgid(0));
            break;

	case 'u': {
    	    struct rlimit rl;
   	    if (getrlimit(RLIMIT_FSIZE, &rl) == 0) {
        	if (rl.rlim_cur == RLIM_INFINITY) {
            	    printf("ulimit=unlimited\n");
        	} else {
            	    printf("ulimit=%ld\n", (long)rl.rlim_cur);
                }
            } else {
                perror("getrlimit");
            }
            break;
	}	

        case 'U': {
            long val = atol(optarg);
            struct rlimit rl;
            if (getrlimit(RLIMIT_FSIZE, &rl) == 0) {
                printf("Current limit: cur=%ld, max=%ld\n", (long)rl.rlim_cur, (long)rl.rlim_max);
                rl.rlim_cur = val;
                if (setrlimit(RLIMIT_FSIZE, &rl) == 0) {
                    printf("ulimit set to %ld\n", val);
                    getrlimit(RLIMIT_FSIZE, &rl);
                    printf("After setting: cur=%ld, max=%ld\n", (long)rl.rlim_cur, (long)rl.rlim_max);
                }
                else {
                    perror("setrlimit");
                }
            }
            else {
                perror("getrlimit");
            }
            break;
    	}

        case 'c': {
            struct rlimit rl;
            if (getrlimit(RLIMIT_CORE, &rl) == 0) {
                printf("core=%ld\n", (long)rl.rlim_cur);
            }
            else {
                perror("getrlimit");
            }
            break;
        }

        case 'C': {
            long val = atol(optarg);
            struct rlimit rl;
            if (getrlimit(RLIMIT_CORE, &rl) == 0) {
                rl.rlim_cur = val;
                if (setrlimit(RLIMIT_CORE, &rl) == 0) {
                    printf("core set to %ld\n", val);
                }
                else {
                    perror("setrlimit");
                }
            }
            else {
                perror("getrlimit");
            }
            break;
        }

        case 'd': {
            char buf[1024];
            if (getcwd(buf, sizeof(buf)) != NULL) {
                printf("cwd=%s\n", buf);
            }
            else {
                perror("getcwd");
            }
            break;
        }

        case 'V':
            if (putenv(optarg) == 0) {
                printf("env set: %s\n", optarg);
            }
            else {
                perror("putenv");
            }
            break;

        case ':':
            fprintf(stderr, "option -%c needs argument\n", optopt);
            break;

        case '?':
            fprintf(stderr, "invalid option: -%c\n", optopt);
            break;
        }
    }

    return 0;
}
