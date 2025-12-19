#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

static const char *name_of(uid_t u) {
    struct passwd *p = getpwuid(u);
    
    if (p != NULL) {
        return p->pw_name;
    } else {
        return "?";
    }
}

void print_user_ids() {
    uid_t real_id = getuid();
    uid_t effective_id = geteuid();
    
    printf("Real ID: %ld (%s)\n", (long)real_id, name_of(real_id));
    printf("Effective ID: %ld (%s)\n", (long)effective_id, name_of(effective_id));
}

void try_open_file(const char *path) {
    printf("Trying to open file: %s\n", path);
    
    FILE *f = fopen(path, "r+");
    if (f) {
        printf("fopen: OK\n");
        fclose(f);
    } else {
        perror("fopen");
    }
}

void make_ids_equal() {
    printf("Making real and effective user IDs equal...\n");
    
    if (setuid(getuid()) != 0) {
        perror("setuid(getuid())");
    } else {
        printf("Successfully set real and effective user IDs to be equal\n");
    }
}

int main(int argc, char *argv[]) {
    const char *path;
    if (argc > 1) {
        path = argv[1];
    } else {
        path = "data.txt";
    }
    
    printf("=== First attempt ===\n");
    print_user_ids();
    try_open_file(path);
    
    printf("\n");
    make_ids_equal();
    
    printf("\n=== Second attempt ===\n");
    print_user_ids();
    try_open_file(path);
    
    return 0;
}
