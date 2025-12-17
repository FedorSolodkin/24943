#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <errno.h>

void print_file_info(const char *filename) {
    struct stat st;
    
    if (lstat(filename, &st) == -1) {
        perror("lstat");
        return;
    }
    
    char file_type;
    if (S_ISDIR(st.st_mode)) file_type = 'd';
    else if (S_ISREG(st.st_mode)) file_type = '-';
    else if (S_ISLNK(st.st_mode)) file_type = 'l';
    else if (S_ISCHR(st.st_mode)) file_type = 'c';
    else if (S_ISBLK(st.st_mode)) file_type = 'b';
    else if (S_ISFIFO(st.st_mode)) file_type = 'p';
    else if (S_ISSOCK(st.st_mode)) file_type = 's';
    else file_type = '?';
    
    char permissions[10];
    permissions[0] = (st.st_mode & S_IRUSR) ? 'r' : '-';
    permissions[1] = (st.st_mode & S_IWUSR) ? 'w' : '-';
    permissions[2] = (st.st_mode & S_IXUSR) ? 'x' : '-';
    permissions[3] = (st.st_mode & S_IRGRP) ? 'r' : '-';
    permissions[4] = (st.st_mode & S_IWGRP) ? 'w' : '-';
    permissions[5] = (st.st_mode & S_IXGRP) ? 'x' : '-';
    permissions[6] = (st.st_mode & S_IROTH) ? 'r' : '-';
    permissions[7] = (st.st_mode & S_IWOTH) ? 'w' : '-';
    permissions[8] = (st.st_mode & S_IXOTH) ? 'x' : '-';
    permissions[9] = '\0';
    
    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr = getgrgid(st.st_gid);
    
    char *owner_name = pw ? pw->pw_name : "unknown";
    char *group_name = gr ? gr->gr_name : "unknown";
    
    char time_buf[20];
    struct tm *timeinfo = localtime(&st.st_mtime);
    strftime(time_buf, sizeof(time_buf), "%b %d  %Y", timeinfo);
    
    char *basename = strrchr(filename, '/');
    if (basename) basename++;
    else basename = (char *)filename;
    
    printf("%c%s %2ld %-8s %-8s ", 
           file_type, permissions, (long)st.st_nlink, owner_name, group_name);
    
    if (S_ISREG(st.st_mode) || S_ISDIR(st.st_mode)) {
        printf("%8ld ", (long)st.st_size);
    } else {
        printf("%8s ", "");
    }
    
    printf("%s %s\n", time_buf, basename);
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        print_file_info(".");
    } else {
        for (int i = 1; i < argc; i++) {
            print_file_info(argv[i]);
        }
    }
    
    return 0;
}
