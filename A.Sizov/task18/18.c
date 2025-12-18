#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <libgen.h>

void print_file_info(const char *path) {
    struct stat st;
    if (lstat(path, &st) == -1) {
        perror(path);
        return;
    }

    char type;
    if (S_ISDIR(st.st_mode)) type = 'd';
    else if (S_ISREG(st.st_mode)) type = '-';
    else type = '?';

    char perms[10];
    perms[0] = (st.st_mode & S_IRUSR) ? 'r' : '-';
    perms[1] = (st.st_mode & S_IWUSR) ? 'w' : '-';
    perms[2] = (st.st_mode & S_IXUSR) ? 'x' : '-';
    perms[3] = (st.st_mode & S_IRGRP) ? 'r' : '-';
    perms[4] = (st.st_mode & S_IWGRP) ? 'w' : '-';
    perms[5] = (st.st_mode & S_IXGRP) ? 'x' : '-';
    perms[6] = (st.st_mode & S_IROTH) ? 'r' : '-';
    perms[7] = (st.st_mode & S_IWOTH) ? 'w' : '-';
    perms[8] = (st.st_mode & S_IXOTH) ? 'x' : '-';
    perms[9] = '\0';

    nlink_t nlinks = st.st_nlink;

    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr = getgrgid(st.st_gid);
    const char *owner = pw ? pw->pw_name : "unknown";
    const char *group = gr ? gr->gr_name : "unknown";

    off_t size = S_ISREG(st.st_mode) ? st.st_size : 0;

    char timebuf[20];
    struct tm *tm = localtime(&st.st_mtime);
    strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", tm);

    char *name = basename((char *)path);

    printf("%c%s %2lu %-8s %-8s", type, perms, (unsigned long)nlinks, owner, group);

    if (S_ISREG(st.st_mode))
        printf(" %8lld", (long long)size);
    else
        printf(" %8s", "");

    printf(" %12s %s\n", timebuf, name);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <файл> [файл...]\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        print_file_info(argv[i]);
    }

    return 0;
}
