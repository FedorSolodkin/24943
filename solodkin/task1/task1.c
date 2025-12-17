#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>


static long custom_ulimit = -1;

int main(int argc, char *argv[]) {
    int opt;
    extern char *optarg;
    extern char **environ;
    
    if (argc == 1) {
        printf("No arguments provided. Use -h for help.\n");
        return 0;
    }
    
    while ((opt = getopt(argc, argv, "ispucdvU:C:V:h")) != -1) {
        switch (opt) {
            case 'i':
                printf("=== User and Group IDs ===\n");
                printf("Real UID: %ld\n", (long)getuid());
                printf("Effective UID: %ld\n", (long)geteuid());
                printf("Real GID: %ld\n", (long)getgid());
                printf("Effective GID: %ld\n", (long)getegid());
                break;
                
            case 's':
                printf("=== Process Group Leader ===\n");
                if (setpgid(0, 0) == 0) {
                    printf("Became process group leader\n");
                    printf("New process group: %ld\n", (long)getpgrp());
                } else {
                    perror("setpgid failed");
                }
                break;
                
            case 'p':
                printf("=== Process IDs ===\n");
                printf("PID: %ld\n", (long)getpid());
                printf("Parent PID: %ld\n", (long)getppid());
                printf("Process group: %ld\n", (long)getpgrp());
                break;
                
            case 'u':
                printf("=== Ulimit (file size limit) ===\n");
                if (custom_ulimit != -1) {
                    printf("Max child processes per user: %ld (custom)\n", custom_ulimit);
                } else {
                    printf("Max child processes per user: %ld (system)\n", sysconf(_SC_CHILD_MAX));
                }
                break;
            
            case 'U':
                printf("=== Change Ulimit (file size limit) ===\n");
                custom_ulimit = atol(optarg);
                if (custom_ulimit < 0) {
                    printf("Error: Invalid ulimit value: %s\n", optarg);
                    custom_ulimit = -1;
                } else {
                    printf("Ulimit changed to: %ld\n", custom_ulimit);
                    
                }
                break;
                
            case 'c': {
                printf("=== Core File Size ===\n");
                struct rlimit rlim;
                if (getrlimit(RLIMIT_CORE, &rlim) == 0) {
                    printf("Core file size: %ld bytes\n", (long)rlim.rlim_cur);
                } else {
                    perror("getrlimit core failed");
                }
                break;
            }
            
            case 'C': {
                printf("=== Change Core File Size ===\n");
                struct rlimit rlim;
                long new_size = atol(optarg);
                
                if (new_size < 0) {
                    printf("Error: Invalid core size: %s\n", optarg);
                    break;
                }
                
                if (getrlimit(RLIMIT_CORE, &rlim) == 0) {
                    rlim.rlim_cur = (rlim_t)new_size;
                    if (setrlimit(RLIMIT_CORE, &rlim) == 0) {
                        printf("Core file size changed to: %ld bytes\n", new_size);
                    } else {
                        perror("setrlimit core failed");
                    }
                } else {
                    perror("getrlimit core failed");
                }
                break;
            }
            
            case 'd': {
                printf("=== Current Directory ===\n");
                char *cwd = getcwd(NULL, 0);
                if (cwd) {
                    printf("Current directory: %s\n", cwd);
                    free(cwd);
                } else {
                    perror("getcwd failed");
                }
                break;
            }
            
            case 'v':
                printf("=== Environment Variables ===\n");
                for (char **env = environ; *env; env++) {
                    printf("%s\n", *env);
                }
                break;
            
            case 'V':
                printf("=== Set Environment Variable ===\n");
                if (putenv(optarg) == 0) {
                    printf("Environment variable set: %s\n", optarg);
                } else {
                    perror("putenv failed");
                }
                break;
            
            case 'h':
                printf("=== Help ===\n");
                printf("Usage: %s [options]\n", argv[0]);
                printf("Options:\n");
                printf("  -i  Print user and group IDs\n");
                printf("  -s  Become process group leader\n");
                printf("  -p  Print process IDs\n");
                printf("  -u  Print ulimit (file size limit)\n");
                printf("  -U <value> Change ulimit (file size limit)\n");
                printf("  -c  Print core file size\n");
                printf("  -C <size> Change core file size\n");
                printf("  -d  Print current directory\n");
                printf("  -v  Print environment variables\n");
                printf("  -V <name=value> Set environment variable\n");
                printf("  -h  This help message\n");
                break;
            
            default:
                fprintf(stderr, "Error: Unknown option\n");
                fprintf(stderr, "Use -h for help\n");
                break;
        }
    }
    
    return 0;
}