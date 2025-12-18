#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void print_usage(const char *progname) {
    fprintf(stderr, "Usage: %s [options]\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -i    Print real and effective user and group IDs\n");
    fprintf(stderr, "  -s    Process becomes group leader\n");
    fprintf(stderr, "  -p    Print process, parent process, and process group IDs\n");
    fprintf(stderr, "  -u    Print ulimit value\n");
    fprintf(stderr, "  -U    Change ulimit value\n");
    fprintf(stderr, "  -c    Print core file size limit in bytes\n");
    fprintf(stderr, "  -C    Change core file size limit\n");
    fprintf(stderr, "  -d    Print current working directory\n");
    fprintf(stderr, "  -v    Print environment variables and their values\n");
    fprintf(stderr, "  -V    Set environment variable (name=value)\n");
}

void print_user_group_ids() {
    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());
    printf("Real GID: %d\n", getgid());
    printf("Effective GID: %d\n", getegid());
}

void become_group_leader() {
    if (setpgid(0, 0) == -1) {
        perror("setpgid");
    } else {
        printf("Process became group leader\n");
    }
}

void print_process_ids() {
    printf("Process ID: %d\n", getpid());
    printf("Parent Process ID: %d\n", getppid());
    printf("Process Group ID: %d\n", getpgrp());
}

void print_ulimit() {
    struct rlimit rlim;
    if (getrlimit(RLIMIT_FSIZE, &rlim) == -1) {
        perror("getrlimit");
        return;
    }
    printf("ulimit: %ld bytes\n", (long)rlim.rlim_cur);
}

int change_ulimit(const char *value) {
    char *endptr;
    long new_limit = strtol(value, &endptr, 10);
    
    if (endptr == value || *endptr != '\0' || new_limit < 0) {
        fprintf(stderr, "Invalid ulimit value: %s\n", value);
        return -1;
    }
    
    struct rlimit rlim;
    rlim.rlim_cur = (rlim_t)new_limit;
    rlim.rlim_max = (rlim_t)new_limit;
    
    if (setrlimit(RLIMIT_FSIZE, &rlim) == -1) {
        perror("setrlimit");
        return -1;
    }
    
    printf("ulimit changed to %ld bytes\n", new_limit);
    return 0;
}

void print_core_size() {
    struct rlimit rlim;
    if (getrlimit(RLIMIT_CORE, &rlim) == -1) {
        perror("getrlimit");
        return;
    }
    printf("core size: %ld bytes\n", (long)rlim.rlim_cur);
}

int change_core_size(const char *value) {
    char *endptr;
    long new_size = strtol(value, &endptr, 10);
    
    if (endptr == value || *endptr != '\0' || new_size < 0) {
        fprintf(stderr, "Invalid core size value: %s\n", value);
        return -1;
    }
    
    struct rlimit rlim;
    rlim.rlim_cur = (rlim_t)new_size;
    rlim.rlim_max = (rlim_t)new_size;
    
    if (setrlimit(RLIMIT_CORE, &rlim) == -1) {
        perror("setrlimit");
        return -1;
    }
    
    printf("core size changed to %ld bytes\n", new_size);
    return 0;
}

void print_current_directory() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        return;
    }
    printf("current directory: %s\n", cwd);
}

void print_environment() {
    extern char **environ;
    char **env;
    
    for (env = environ; *env != NULL; env++) {
        printf("%s\n", *env);
    }
}

int set_environment_variable(const char *input) {
    char *equals = strchr(input, '=');
    if (equals == NULL || equals == input) {
        fprintf(stderr, "Invalid format for -V. Use: name=value\n");
        return -1;
    }
    
    size_t name_len = equals - input;
    char *name = malloc(name_len + 1);
    if (name == NULL) {
        perror("malloc");
        return -1;
    }
    
    strncpy(name, input, name_len);
    name[name_len] = '\0';
    
    const char *value = equals + 1;
    
    if (setenv(name, value, 1) == -1) {
        perror("setenv");
        free(name);
        return -1;
    }
    
    printf("Environment variable %s set to %s\n", name, value);
    free(name);
    return 0;
}

int process_option(int opt, const char *optarg_value) {
    switch (opt) {
        case 'i':
            print_user_group_ids();
            break;
        case 's':
            become_group_leader();
            break;
        case 'p':
            print_process_ids();
            break;
        case 'u':
            print_ulimit();
            break;
        case 'U':
            if (optarg_value == NULL) {
                fprintf(stderr, "Option -U requires an argument\n");
                return -1;
            }
            if (change_ulimit(optarg_value) == -1) {
                return -1;
            }
            break;
        case 'c':
            print_core_size();
            break;
        case 'C':
            if (optarg_value == NULL) {
                fprintf(stderr, "Option -C requires an argument\n");
                return -1;
            }
            if (change_core_size(optarg_value) == -1) {
                return -1;
            }
            break;
        case 'd':
            print_current_directory();
            break;
        case 'v':
            print_environment();
            break;
        case 'V':
            if (optarg_value == NULL) {
                fprintf(stderr, "Option -V requires an argument\n");
                return -1;
            }
            if (set_environment_variable(optarg_value) == -1) {
                return -1;
            }
            break;
        case '?':
        default:
            return -1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int i;
    
    if (argc == 1) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    
    char **args_copy = malloc((argc + 1) * sizeof(char *));
    if (args_copy == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }
    
    for (i = 0; i < argc; i++) {
        args_copy[i] = argv[i];
    }
    args_copy[argc] = NULL;
    
    optind = 1;
    
    int result = EXIT_SUCCESS;
    
    struct {
        int opt;
        char *optarg;
    } *options = malloc((argc - 1) * sizeof(*options));
    
    if (options == NULL) {
        perror("malloc");
        free(args_copy);
        return EXIT_FAILURE;
    }
    
    int opt_count = 0;
    
    while (1) {
        int opt = getopt(argc, args_copy, "ispuU:cC:dvV:");
        if (opt == -1) {
            break;
        }
        
        options[opt_count].opt = opt;
        options[opt_count].optarg = (optarg != NULL) ? strdup(optarg) : NULL;
        opt_count++;
    }
    
    for (i = opt_count - 1; i >= 0; i--) {
        if (process_option(options[i].opt, options[i].optarg) == -1) {
            result = EXIT_FAILURE;
        }
        if (options[i].optarg != NULL) {
            free(options[i].optarg);
        }
    }
    
    free(options);
    free(args_copy);
    
    return result;
}
