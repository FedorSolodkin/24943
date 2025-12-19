#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <libgen.h>

void display_info(char* path){
    struct stat file;
    if (stat(path, &file) != 0){
        perror(path);
        return;
    }

    char file_type;

    if (S_ISDIR(file.st_mode)) {
        file_type = 'd';
    } else if (S_ISREG(file.st_mode)) {
        file_type = '-';
    } else {
        file_type = '?';
    }

    char rights[10];
    rights[0] = (file.st_mode & S_IRUSR) ? 'r' : '-';
    rights[1] = (file.st_mode & S_IWUSR) ? 'w' : '-';
    rights[2] = (file.st_mode & S_IXUSR) ? 'x' : '-';
    rights[3] = (file.st_mode & S_IRGRP) ? 'r' : '-';
    rights[4] = (file.st_mode & S_IWGRP) ? 'w' : '-';
    rights[5] = (file.st_mode & S_IXGRP) ? 'x' : '-';
    rights[6] = (file.st_mode & S_IROTH) ? 'r' : '-';
    rights[7] = (file.st_mode & S_IWOTH) ? 'w' : '-';
    rights[8] = (file.st_mode & S_IXOTH) ? 'x' : '-';
    rights[9] = '\0';

    struct passwd* owner = getpwuid(file.st_uid);
    struct group* grp = getgrgid(file.st_gid);
    const char* username = (owner != NULL) ? owner->pw_name : "nobody";
    const char* groupname = (grp != NULL) ? grp->gr_name : "nobody";

    char size_field[16] = "";
    if (S_ISREG(file.st_mode)) {
        snprintf(size_field, sizeof(size_field), "%lld", (long long)file.st_size);
    }

    char time_buf[20];
    struct tm* tm_info = localtime(&file.st_mtime);
    strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", tm_info);

    char* filename = basename((char*)path);

    printf("%c%s %2ld %-10s %-10s %10s %s %s\n",
           file_type,
           rights,
           (long)file.st_nlink,
           username,
           groupname,
           size_field,
           time_buf,
           filename);
}

int main(int argc, char *argv[]){
    if (argc < 2){
        display_info(".");
    }else{
        for (int i = 1; i < argc; i++) {
            display_info(argv[i]);
        }       
    }
    return 0;
}
