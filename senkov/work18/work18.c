#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

void print_file_info(const char *path) {
    struct stat sb;
    if (stat(path, &sb) == -1) {
        perror(path);
        return;
    }

    // Тип файла
    char type = '?';
    if (S_ISDIR(sb.st_mode)) type = 'd';
    else if (S_ISREG(sb.st_mode)) type = '-';

    // Права доступа
    char perms[10] = "---------";
    perms[0] = (sb.st_mode & S_IRUSR) ? 'r' : '-';
    perms[1] = (sb.st_mode & S_IWUSR) ? 'w' : '-';
    perms[2] = (sb.st_mode & S_IXUSR) ? 'x' : '-';
    perms[3] = (sb.st_mode & S_IRGRP) ? 'r' : '-';
    perms[4] = (sb.st_mode & S_IWGRP) ? 'w' : '-';
    perms[5] = (sb.st_mode & S_IXGRP) ? 'x' : '-';
    perms[6] = (sb.st_mode & S_IROTH) ? 'r' : '-';
    perms[7] = (sb.st_mode & S_IWOTH) ? 'w' : '-';
    perms[8] = (sb.st_mode & S_IXOTH) ? 'x' : '-';

    // Имя владельца и группы
    struct passwd *pw = getpwuid(sb.st_uid);
    struct group *gr = getgrgid(sb.st_gid);
    const char *owner = pw ? pw->pw_name : "";
    const char *group = gr ? gr->gr_name : "";

    // Размер файла (если обычный файл)
    long long size = (S_ISREG(sb.st_mode)) ? sb.st_size : 0;

    // Дата модификации
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M", localtime(&sb.st_mtime));

    // Имя файла (без пути, если был указан путь)
    const char *filename = strrchr(path, '/');
    if (filename) filename++;
    else filename = path;

    // Вывод в формате таблицы
    printf("%c%s %2lu %-8s %-8s %8lld %s %s\n",
           type, perms,
           (long)sb.st_nlink,
           owner, group,
           size,
           time_str,
           filename);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <файл1> [файл2] ...\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        print_file_info(argv[i]);
    }

    return 0;
}
