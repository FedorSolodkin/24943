#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

typedef enum {
    OPT_SUCCESS = 0,
    OPT_ERROR_INVALID_VALUE = -1,
    OPT_ERROR_NEGATIVE_VALUE = -2,
    OPT_ERROR_MISSING_ARGUMENT = -3,
    OPT_ERROR_INVALID_FORMAT = -4
} OptionResult;

typedef struct {
    int opt;
    char *arg;
} Option;

long safe_strtol(const char *str, const char *option_name) {
    if (!str) {return OPT_ERROR_MISSING_ARGUMENT;}

    char *endptr;
    errno = 0;
    long val = strtol(str, &endptr, 10);

    if (errno != 0) {
        fprintf(stderr, "Error converting '%s' for %s: ", str, option_name);
        perror("");
        return OPT_ERROR_INVALID_VALUE;
    }
    
    if (*endptr != '\0') {
        fprintf(stderr, "Invalid numeric value for %s: '%s'\n", option_name, str);
        return OPT_ERROR_INVALID_VALUE;
    }
    
    if (val < 0) {
        fprintf(stderr, "Value for %s must be non-negative: '%s'\n", option_name, str);
        return OPT_ERROR_NEGATIVE_VALUE;
    }
    
    return val;
}

void free_options(Option *opts, int count) {
    if (!opts) return;
    
    for (int i = 0; i < count; i++) {free(opts[i].arg);}
    free(opts);
}

void print_error(const char *context, OptionResult result) {
    const char *message = "Unknown error";
    
    switch (result) {
        case OPT_ERROR_MISSING_ARGUMENT:
            message = "Missing required argument";
            break;
        case OPT_ERROR_INVALID_VALUE:
            message = "Invalid value";
            break;
        case OPT_ERROR_NEGATIVE_VALUE:
            message = "Value must be non-negative";
            break;
        case OPT_ERROR_INVALID_FORMAT:
            message = "Invalid format";
            break;
        default:
            return; 
    }
    
    fprintf(stderr, "%s: %s\n", context, message);
}

OptionResult handle_i(void) {
    uid_t ruid = getuid();
    uid_t euid = geteuid();
    gid_t rgid = getgid();
    gid_t egid = getegid();
    
    printf("Real UID: %d, Effective UID: %d\n", ruid, euid);
    printf("Real GID: %d, Effective GID: %d\n", rgid, egid);
    
    return OPT_SUCCESS;
}

OptionResult handle_s(void) {
    if (setpgid(0, 0) == -1) {
        perror("setpgid");
        return OPT_ERROR_INVALID_VALUE;
    }
    
    printf("Process became process group leader.\n");
    return OPT_SUCCESS;
}

OptionResult handle_p(void) {
    pid_t pid = getpid();
    pid_t ppid = getppid();
    pid_t pgid = getpgid(0);
    
    printf("PID: %d, PPID: %d, PGID: %d\n", pid, ppid, pgid);
    return OPT_SUCCESS;
}

OptionResult handle_u(void) {
    struct rlimit rl;
    
    // В Solaris используем RLIMIT_NOFILE вместо RLIMIT_NPROC
    if (getrlimit(RLIMIT_NOFILE, &rl) != 0) {
        perror("getrlimit");
        return OPT_ERROR_INVALID_VALUE;
    }
    
    printf("Maximum number of open files (ulimit -n): %ld\n", (long)rl.rlim_cur);
    return OPT_SUCCESS;
}

OptionResult handle_c(void) {
    struct rlimit rl;
    
    if (getrlimit(RLIMIT_CORE, &rl) != 0) {
        perror("getrlimit");
        return OPT_ERROR_INVALID_VALUE;
    }
    
    printf("Core file size: %ld bytes\n", (long)rl.rlim_cur);
    return OPT_SUCCESS;
}

OptionResult handle_U(const char *arg) {
    if (!arg) {return OPT_ERROR_MISSING_ARGUMENT;}
    
    long size = safe_strtol(arg, "ulimit option");
    if (size < 0) { return (OptionResult)size;}
    
    struct rlimit rl = {
        .rlim_cur = (rlim_t)size,
        .rlim_max = (rlim_t)size
    };
    
    // В Solaris изменяем лимит открытых файлов
    if (setrlimit(RLIMIT_NOFILE, &rl) == -1) {
        perror("setrlimit");
        return OPT_ERROR_INVALID_VALUE;
    }
    
    printf("Maximum open files set to %ld\n", size);
    return OPT_SUCCESS;
}

OptionResult handle_C(const char *arg) {
    if (!arg) {return OPT_ERROR_MISSING_ARGUMENT;}
    
    long size = safe_strtol(arg, "core limit option");
    if (size < 0) { return (OptionResult)size;}
    
    struct rlimit rl = {
        .rlim_cur = (rlim_t)size,
        .rlim_max = (rlim_t)size
    };
    
    if (setrlimit(RLIMIT_CORE, &rl) == -1) {
        perror("setrlimit");
        return OPT_ERROR_INVALID_VALUE;
    }
    
    printf("Core file size set to %ld bytes.\n", size);
    return OPT_SUCCESS;
}

OptionResult handle_d(void) {
    char cwd[PATH_MAX];
    
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        return OPT_ERROR_INVALID_VALUE;
    }
    
    printf("Current working directory: %s\n", cwd);
    return OPT_SUCCESS;
}

OptionResult handle_v(void) {
    extern char **environ;
    
    for (char **env = environ; *env != NULL; env++) {printf("%s\n", *env);}
    
    return OPT_SUCCESS;
}

OptionResult handle_V(const char *arg) {
    if (!arg) {return OPT_ERROR_MISSING_ARGUMENT;}
    
    char *eq = strchr(arg, '=');
    if (!eq || eq == arg) {return OPT_ERROR_INVALID_FORMAT;}
    
    char *arg_copy = strdup(arg);
    if (!arg_copy) {
        perror("strdup");
        return OPT_ERROR_INVALID_VALUE;
    }
    
    eq = strchr(arg_copy, '=');
    *eq = '\0';
    
    const char *name = arg_copy;
    const char *value = eq + 1;
    
    OptionResult result = OPT_SUCCESS;
    if (setenv(name, value, 1) == -1) {
        perror("setenv");
        result = OPT_ERROR_INVALID_VALUE;
    } else {
        printf("Environment variable set: %s=%s\n", name, value);
    }
    
    free(arg_copy);
    return result;
}

int main(int argc, char *argv[]) {
    Option *opts = NULL;
    int opt_count = 0;
    int opt_capacity = 10;
    int exit_code = EXIT_SUCCESS;

    opts = malloc(opt_capacity * sizeof(Option));
    if (!opts) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    int opt;
    while ((opt = getopt(argc, argv, "ispucC:U:d:vV:")) != -1) {
        if (opt_count >= opt_capacity) {
            opt_capacity *= 2;
            Option *tmp = realloc(opts, opt_capacity * sizeof(Option));
            if (!tmp) {
                perror("realloc");
                free_options(opts, opt_count);
                exit(EXIT_FAILURE);
            }
            opts = tmp;
        }
        
        opts[opt_count].opt = opt;
        opts[opt_count].arg = optarg ? strdup(optarg) : NULL;
        opt_count++;
    }

    for (int i = opt_count - 1; i >= 0; i--) {
        OptionResult result = OPT_SUCCESS;
        const char *option_name = NULL;
        
        switch (opts[i].opt) {
            case 'i': 
                result = handle_i(); 
                option_name = "-i";
                break;
            case 's': 
                result = handle_s(); 
                option_name = "-s";
                break;
            case 'p': 
                result = handle_p(); 
                option_name = "-p";
                break;
            case 'u': 
                result = handle_u(); 
                option_name = "-u";
                break;
            case 'c': 
                result = handle_c(); 
                option_name = "-c";
                break;
            case 'U': 
                result = handle_U(opts[i].arg); 
                option_name = "-U";
                break;
            case 'C': 
                result = handle_C(opts[i].arg); 
                option_name = "-C";
                break;
            case 'd': 
                result = handle_d(); 
                option_name = "-d";
                break;
            case 'v': 
                result = handle_v(); 
                option_name = "-v";
                break;
            case 'V': 
                result = handle_V(opts[i].arg); 
                option_name = "-V";
                break;
            case '?':
                exit_code = EXIT_FAILURE;
                continue;
            default:
                fprintf(stderr, "Unknown option: %c\n", opts[i].opt);
                exit_code = EXIT_FAILURE;
                continue;
        }
        
        if (result != OPT_SUCCESS && option_name) {
            print_error(option_name, result);
            exit_code = EXIT_FAILURE;
        }
    }

    free_options(opts, opt_count);
    return exit_code;
}