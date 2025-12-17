#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <libgen.h>


const char *months[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};


int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s filename\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        struct stat file_st;
        if (stat(argv[i], &file_st) == -1) {
            perror(argv[i]);
            continue;
        }

        if (S_ISDIR(file_st.st_mode)) {
            printf("d");
        } else if (S_ISREG(file_st.st_mode)) {
            printf("-");
        } else {
            printf("?");
        }

        printf("%c", (file_st.st_mode & S_IRUSR) ? 'r' : '-');
        printf("%c", (file_st.st_mode & S_IWUSR) ? 'w' : '-');
        printf("%c", (file_st.st_mode & S_IXUSR) ? 'x' : '-');
        printf("%c", (file_st.st_mode & S_IRGRP) ? 'r' : '-');
        printf("%c", (file_st.st_mode & S_IWGRP) ? 'w' : '-');
        printf("%c", (file_st.st_mode & S_IXGRP) ? 'x' : '-');
        printf("%c", (file_st.st_mode & S_IROTH) ? 'r' : '-');
        printf("%c", (file_st.st_mode & S_IWOTH) ? 'w' : '-');
        printf("%c", (file_st.st_mode & S_IXOTH) ? 'x' : '-');

        printf(" %-2lu ", file_st.st_nlink);

        struct passwd *user = getpwuid(file_st.st_uid);
        struct group  *group = getgrgid(file_st.st_gid);
        if (user) printf("%-8s ", user->pw_name);
        else printf("%-8u", file_st.st_uid);
        if (group) printf("%-8s ", group->gr_name);
        else printf("%-8u", file_st.st_gid);

        printf(" %5lld ", (long long)file_st.st_size);

        struct tm *mt = localtime(&file_st.st_mtime);
        printf("%s %02d %02d:%02d ",
               months[mt->tm_mon], mt->tm_mday, mt->tm_hour, mt->tm_min);

        printf("%s\n", basename(argv[i]));
    }

    return 0;
}