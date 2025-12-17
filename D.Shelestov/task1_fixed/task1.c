#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

void print_user_group_ids() {
    printf("=== User and Group IDs ===\n");
    printf("Real User ID (RUID): %d\n", getuid());
    printf("Effective User ID (EUID): %d\n", geteuid());
    printf("Real Group ID (RGID): %d\n", getgid());
    printf("Effective Group ID (EGID): %d\n", getegid());
}

void become_process_group_leader() {
    printf("=== Process Group Leadership ===\n");
    if (setpgid(0, 0) == 0) {
        printf("Became new process group leader. New PGID: %d\n", getpgrp());
    } else {
        perror("Error setting group leader");
    }
}

void print_process_ids() {
    printf("=== Process Identification ===\n");
    printf("Process ID (PID): %d\n", getpid());
    printf("Parent Process ID (PPID): %d\n", getppid());
    printf("Process Group ID (PGID): %d\n", getpgrp());
}

void print_process_limit() {
    printf("=== Process Limit (ulimit -u) ===\n");
    struct rlimit rlim;
    
    if (getrlimit(RLIMIT_CPU, &rlim) == 0) {
        printf("Max processes: ");
        if (rlim.rlim_cur == RLIM_INFINITY) {
            printf("unlimited\n");
        } else {
            printf("%ld\n", (long)rlim.rlim_cur);
        }
    } else {
        perror("Error getting process limit");
    }
}

void print_file_size_limit() {
    printf("=== File Size Limit (ulimit -f) ===\n");
    struct rlimit rlim;
    
    if (getrlimit(RLIMIT_FSIZE, &rlim) == 0) {
        printf("Max file size: ");
        if (rlim.rlim_cur == RLIM_INFINITY) {
            printf("unlimited\n");
        } else {
            printf("%ld bytes\n", (long)rlim.rlim_cur);
        }
    } else {
        perror("Error getting file size limit");
    }
}

void print_data_size_limit() {
    printf("=== Data Size Limit (ulimit -d) ===\n");
    struct rlimit rlim;
    
    if (getrlimit(RLIMIT_DATA, &rlim) == 0) {
        printf("Max data segment size: ");
        if (rlim.rlim_cur == RLIM_INFINITY) {
            printf("unlimited\n");
        } else {
            printf("%ld bytes\n", (long)rlim.rlim_cur);
        }
    } else {
        perror("Error getting data limit");
    }
}

void print_stack_size_limit() {
    printf("=== Stack Size Limit (ulimit -s) ===\n");
    struct rlimit rlim;
    
    if (getrlimit(RLIMIT_STACK, &rlim) == 0) {
        printf("Max stack size: ");
        if (rlim.rlim_cur == RLIM_INFINITY) {
            printf("unlimited\n");
        } else {
            printf("%ld bytes\n", (long)rlim.rlim_cur);
        }
    } else {
        perror("Error getting stack limit");
    }
}

int set_file_size_limit(const char *value) {
    printf("=== Setting File Size Limit ===\n");
    struct rlimit rlim;
    long new_limit;
    char *endptr;
    
    new_limit = strtol(value, &endptr, 10);
    
    if (endptr == value) {
        fprintf(stderr, "Error: '%s' is not a number\n", value);
        return -1;
    }
    if (*endptr != '\0') {
        fprintf(stderr, "Error: non-numeric chars in '%s'\n", value);
        return -1;
    }
    if (new_limit < 0) {
        fprintf(stderr, "Error: ulimit cannot be negative\n");
        return -1;
    }
    
    if (getrlimit(RLIMIT_FSIZE, &rlim) != 0) {
        perror("Error getting current ulimit");
        return -1;
    }
    
    rlim.rlim_cur = (rlim_t)new_limit;
    if (setrlimit(RLIMIT_FSIZE, &rlim) != 0) {
        perror("Error setting ulimit");
        return -1;
    }
    
    printf("File size limit set: %ld bytes\n", new_limit);
    return 0;
}

int set_process_limit(const char *value) {
    printf("=== Setting Process Limit ===\n");
    struct rlimit rlim;
    long new_limit;
    char *endptr;
    
    new_limit = strtol(value, &endptr, 10);
    
    if (endptr == value) {
        fprintf(stderr, "Error: '%s' is not a number\n", value);
        return -1;
    }
    if (*endptr != '\0') {
        fprintf(stderr, "Error: non-numeric chars in '%s'\n", value);
        return -1;
    }
    if (new_limit < 0) {
        fprintf(stderr, "Error: process limit cannot be negative\n");
        return -1;
    }
    
    if (getrlimit(RLIMIT_CPU, &rlim) != 0) {
        perror("Error getting current process limit");
        return -1;
    }
    
    rlim.rlim_cur = (rlim_t)new_limit;
    if (setrlimit(RLIMIT_CPU, &rlim) != 0) {
        perror("Error setting process limit");
        return -1;
    }
    
    printf("Process limit set: %ld\n", new_limit);
    return 0;
}

void print_core_size() {
    printf("=== Core File Size Limit (ulimit -c) ===\n");
    struct rlimit rlim;
    
    if (getrlimit(RLIMIT_CORE, &rlim) == 0) {
        printf("Max core file size: ");
        if (rlim.rlim_cur == RLIM_INFINITY) {
            printf("unlimited\n");
        } else {
            printf("%ld bytes\n", (long)rlim.rlim_cur);
        }
    } else {
        perror("Error getting core file size");
    }
}

int set_core_size(const char *value) {
    printf("=== Setting Core File Size ===\n");
    struct rlimit rlim;
    long new_size;
    char *endptr;
    
    new_size = strtol(value, &endptr, 10);
    
    if (endptr == value) {
        fprintf(stderr, "Error: '%s' is not a number\n", value);
        return -1;
    }
    if (*endptr != '\0') {
        fprintf(stderr, "Error: non-numeric chars in '%s'\n", value);
        return -1;
    }
    if (new_size < 0) {
        fprintf(stderr, "Error: core file size cannot be negative\n");
        return -1;
    }
    
    if (getrlimit(RLIMIT_CORE, &rlim) != 0) {
        perror("Error getting current core size");
        return -1;
    }
    
    rlim.rlim_cur = (rlim_t)new_size;
    if (setrlimit(RLIMIT_CORE, &rlim) != 0) {
        perror("Error setting core size");
        return -1;
    }
    
    printf("Core file size set: %ld bytes\n", new_size);
    return 0;
}

void print_environment() {
    printf("=== Environment Variables ===\n");
    extern char **environ;
    
    if (environ == NULL || environ[0] == NULL) {
        printf("No environment variables\n");
        return;
    }
    
    char **env = environ;
    int count = 0;
    
    while (*env != NULL && count < 10) {
        printf("%d: %s\n", ++count, *env);
        env++;
    }
    if (*env != NULL) {
        printf("... (more variables)\n");
    }
}

int set_environment_variable(const char *name_value) {
    printf("=== Setting Environment Variable ===\n");
    
    if (name_value == NULL || strlen(name_value) == 0) {
        fprintf(stderr, "Error: empty env var string\n");
        return -1;
    }
    
    char *equals = strchr(name_value, '=');
    if (equals == NULL || equals == name_value) {
        fprintf(stderr, "Error: format must be 'name=value', got: '%s'\n", 
                name_value ? name_value : "NULL");
        return -1;
    }
    
    size_t name_len = equals - name_value;
    char *name = malloc(name_len + 1);
    if (name == NULL) {
        perror("Memory allocation error");
        return -1;
    }
    
    strncpy(name, name_value, name_len);
    name[name_len] = '\0';
    char *value = equals + 1;
    
    if (setenv(name, value, 1) != 0) {
        perror("Error setting env var");
        free(name);
        return -1;
    }
    
    printf("Env var set: %s=%s\n", name, value);
    free(name);
    return 0;
}

typedef struct {
    char option;
    char *argument;
} command_option_t;

int parse_options_right_to_left(int argc, char *argv[], command_option_t **options) {
    *options = malloc(argc * sizeof(command_option_t));
    if (*options == NULL) {
        perror("Memory allocation error");
        return -1;
    }
    
    int count = 0;
    
    for (int i = argc - 1; i >= 1; i--) {
        if (argv[i][0] != '-' || strlen(argv[i]) != 2) {
            continue;
        }
        
        char opt = argv[i][1];
        
        if (strchr("ispucdvf", opt) != NULL) {
            (*options)[count].option = opt;
            (*options)[count].argument = NULL;
            count++;
        }
        else if (opt == 'U' || opt == 'C' || opt == 'V' || opt == 'P' || opt == 'F' || opt == 'D' || opt == 'S') {
            if (i + 1 >= argc || argv[i + 1][0] == '-') {
                fprintf(stderr, "Error: option -%c requires argument\n", opt);
                continue;
            }
            
            (*options)[count].option = opt;
            (*options)[count].argument = argv[i + 1];
            count++;
            i--;
        }
        else {
            fprintf(stderr, "Warning: unknown option -%c\n", opt);
        }
    }
    
    return count;
}

int main(int argc, char *argv[]) {
    command_option_t *options = NULL;
    int option_count = parse_options_right_to_left(argc, argv, &options);
    
    if (option_count < 0) {
        fprintf(stderr, "Error parsing options\n");
        return 1;
    }
    
    printf("Processing %d options right to left...\n\n", option_count);

    for (int i = 0; i < option_count; i++) {
        switch (options[i].option) {
            case 'i':
                print_user_group_ids();
                break;
            case 's':
                become_process_group_leader();
                break;
            case 'p':
                print_process_ids();
                break;
            case 'u':
                print_process_limit();
                break;
            case 'f':
                print_file_size_limit();
                break;
            case 'd':
                print_data_size_limit();
                break;
            case 'c':
                print_core_size();
                break;
            case 'U':
                if (options[i].argument != NULL) {
                    set_process_limit(options[i].argument);
                } else {
                    fprintf(stderr, "Error: -U requires value\n");
                }
                break;
            case 'F':
                if (options[i].argument != NULL) {
                    set_file_size_limit(options[i].argument);
                } else {
                    fprintf(stderr, "Error: -F requires value\n");
                }
                break;
            case 'D':
                if (options[i].argument != NULL) {
                    printf("Setting data limit: %s\n", options[i].argument);
                } else {
                    fprintf(stderr, "Error: -D requires value\n");
                }
                break;
            case 'C':
                if (options[i].argument != NULL) {
                    set_core_size(options[i].argument);
                } else {
                    fprintf(stderr, "Error: -C requires value\n");
                }
                break;
            case 'S':
                if (options[i].argument != NULL) {
                    printf("Setting stack limit: %s\n", options[i].argument);
                } else {
                    fprintf(stderr, "Error: -S requires value\n");
                }
                break;
            case 'v':
                print_environment();
                break;
            case 'V':
                if (options[i].argument != NULL) {
                    set_environment_variable(options[i].argument);
                } else {
                    fprintf(stderr, "Error: -V requires value\n");
                }
                break;
            default:
                fprintf(stderr, "Unknown option: -%c\n", options[i].option);
                break;
        }
        printf("\n");
    }

    free(options);
    return 0;
}
