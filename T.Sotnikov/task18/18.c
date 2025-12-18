#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <libgen.h>

void process_file(const char *path) {
    struct stat st;

    if (stat(path, &st) == -1) {
        perror(path);
        return;
    }

    char type = '?';
    if (S_ISDIR(st.st_mode)) type = 'd';
    else if (S_ISREG(st.st_mode)) type = '-';

    char perms[10];
    const char *modes = "rwxrwxrwx";
    for (int i = 0; i < 9; i++) {
        perms[i] = (st.st_mode & (1 << (8 - i))) ? modes[i] : '-';
    }
    perms[9] = '\0';

    long links = (long)st.st_nlink;

    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);
    const char *user_name = pw ? pw->pw_name : "unknown";
    const char *group_name = gr ? gr->gr_name : "unknown";

    char size_str[20] = "";
    if (S_ISREG(st.st_mode)) {
        sprintf(size_str, "%ld", (long)st.st_size);
    }

    char date_str[20];
    struct tm *tm_info = localtime(&st.st_mtime);
    strftime(date_str, sizeof(date_str), "%b %d %H:%M", tm_info);

    const char *filename = strrchr(path, '/');
    if (filename) filename++;
    else filename = path;

    printf("%c%s %3ld %-8s %-8s %8s %s %s\n",
           type, perms, links, user_name, group_name, size_str, date_str, filename);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <файл1> [файл2 ...]\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        process_file(argv[i]);
    }

    return 0;
}