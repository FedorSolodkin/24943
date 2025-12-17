#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>

#define MAX_FILENAME_LENGTH 256
#define BUFFER_SIZE (MAX_FILENAME_LENGTH + 1)

char *size_t_to_string(size_t size) {
    static char buffer[20];
    sprintf(buffer, "%zu", size);
    return buffer;
}

void print_file_info(const char *filepath) {
    struct stat file_stat;
    
    // Получаем информацию о файле
    if (stat(filepath, &file_stat) == -1) {
        perror("stat");
        return;
    }

    char file_type;
    if (S_ISDIR(file_stat.st_mode)) {
        file_type = 'd';
    } else if (S_ISREG(file_stat.st_mode)) {
        file_type = '-';
    } else {
        file_type = '?';
    }

    // Определяем права доступа
    char permissions[10];
    permissions[0] = file_type;
    permissions[1] = (file_stat.st_mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (file_stat.st_mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (file_stat.st_mode & S_IXUSR) ? 'x' : '-';

    permissions[4] = (file_stat.st_mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (file_stat.st_mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (file_stat.st_mode & S_IXGRP) ? 'x' : '-';

    permissions[7] = (file_stat.st_mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (file_stat.st_mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (file_stat.st_mode & S_IXOTH) ? 'x' : '-';
    permissions[10] = '\0'; 

    // Получаем информацию о владельце и группе
    struct passwd *pw = getpwuid(file_stat.st_uid);
    struct group *gr = getgrgid(file_stat.st_gid);

    // Получаем дату модификации файла
    char mod_time[20];
    struct tm *timeinfo = localtime(&file_stat.st_mtime);
    strftime(mod_time, sizeof(mod_time), "%Y-%m-%d %H:%M", timeinfo);

    const char *filename = strrchr(filepath, '/');
    filename = filename ? filename + 1 : filepath; // Если путь содержит '/', берем часть после него

    printf("%-15s %-10s %-10s %-8s %4lu %-25s %s\n",
           permissions,
           pw ? pw->pw_name : "unknown",
           gr ? gr->gr_name : "unknown",
           S_ISREG(file_stat.st_mode) ? size_t_to_string(file_stat.st_size) : "",
           (unsigned long)file_stat.st_nlink,
           mod_time,
           filename);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1> <file2> ... <fileN>\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("%-15s %-10s %-10s %-8s %4s %-25s %s\n",
           "Permissions", "Owner", "Group", "Size", "Links", "Modification Time", "Name");
    printf("--------------------------------------------------------------------------------\n");

    for (int i = 1; i < argc; i++) {
        print_file_info(argv[i]);
    }

    return EXIT_SUCCESS;
}