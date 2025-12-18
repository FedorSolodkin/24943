#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void print_file(const char* path) {
    struct stat st;
    if (stat(path, &st) == -1) return;

    char type = '?';
    if (S_ISDIR(st.st_mode)) type = 'd';
    else if (S_ISREG(st.st_mode)) type = '-';

    char perm[10] = "---------";
    if (st.st_mode & S_IRUSR) perm[0] = 'r';
    if (st.st_mode & S_IWUSR) perm[1] = 'w';
    if (st.st_mode & S_IXUSR) perm[2] = 'x';
    if (st.st_mode & S_IRGRP) perm[3] = 'r';
    if (st.st_mode & S_IWGRP) perm[4] = 'w';
    if (st.st_mode & S_IXGRP) perm[5] = 'x';
    if (st.st_mode & S_IROTH) perm[6] = 'r';
    if (st.st_mode & S_IWOTH) perm[7] = 'w';
    if (st.st_mode & S_IXOTH) perm[8] = 'x';

    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);
    const char *uname = pw ? pw->pw_name : "?";
    const char *gname = gr ? gr->gr_name : "?";

    char size_str[12];
    snprintf(size_str, sizeof(size_str), "%ld", (long)(st.st_blocks * 512));

    char time_str[20];
    strftime(time_str, sizeof(time_str), "%b %d %H:%M", localtime(&st.st_mtime));

    const char *name = strrchr(path, '/');
    name = name ? name + 1 : path;

    printf("%c%s %3ld %-8s %-8s %8s %s %s\n",
           type, perm,
           (long)st.st_nlink,
           uname, gname,
           size_str, time_str, name);
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        print_file(".");
    } else {
        for (int i = 1; i < argc; i++)
            print_file(argv[i]);
    }
    return 0;
}
