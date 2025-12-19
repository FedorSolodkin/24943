#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>

void display_uids(const char *phase) {
    uid_t real = getuid();
    uid_t effective = geteuid();

    struct passwd *real_pw = getpwuid(real);
    struct passwd *eff_pw = getpwuid(effective);

    printf("\n=== %s ===\n", phase);
    printf("Real UID      : %d (%s)\n", (int)real, real_pw ? real_pw->pw_name : "n/a");
    printf("Effective UID : %d (%s)\n", (int)effective, eff_pw ? eff_pw->pw_name : "n/a");
}

int try_access_file(const char *filepath) {
    struct stat file_info;
    if (stat(filepath, &file_info) != 0) {
        perror("stat (file check)");
        return -1;
    }

    if (!S_ISREG(file_info.st_mode)) {
        fprintf(stderr, "Error: '%s' is not a regular file.\n", filepath);
        return -1;
    }

    // Попытка открытия
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        perror("fopen failed");
        return -1;
    }

    printf("Successfully opened: %s\n", filepath);
    fclose(fp);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <protected_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *target = argv[1];

    display_uids("Initial State");
    printf("Trying to access file...\n");
    try_access_file(target);

    uid_t original_real_uid = getuid();
    if (setuid(original_real_uid) != 0) {
        perror("setuid() failed");
        exit(EXIT_FAILURE);
    }

    display_uids("After Dropping Privileges");
    printf("Trying to access file again...\n");
    try_access_file(target);

    return 0;
}
