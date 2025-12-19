#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>

void print_file_info(const char *filepath) {
    struct stat file_stat;
    
    // Получаем информацию о файле
    if (stat(filepath, &file_stat) == -1) {
        perror("stat");
        return;
    }

    // Определяем тип файла
    char file_type;
    if (S_ISDIR(file_stat.st_mode)) {
        file_type = 'd';
    } else if (S_ISREG(file_stat.st_mode)) {
        file_type = '-';
    } else {
        file_type = '?';
    }

    // Определяем права доступа
    char permissions[11];
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
    strftime(mod_time, sizeof(mod_time), "%b %d %H:%M", timeinfo);

    // Извлекаем только имя файла из пути
    const char *filename = strrchr(filepath, '/');
    filename = filename ? filename + 1 : filepath;

    // ВАЖНО: Правильное форматирование вывода
    // Формат: permissions links owner group size date name
    printf("%s %2lu %-8s %-8s ",
           permissions,
           (unsigned long)file_stat.st_nlink,
           pw ? pw->pw_name : "unknown",
           gr ? gr->gr_name : "unknown");

    // Выводим размер с правильным выравниванием
    if (S_ISREG(file_stat.st_mode)) {
        // Для обычных файлов выводим размер
        printf("%8lu ", (unsigned long)file_stat.st_size);
    } else {
        // Для каталогов и других типов файлов оставляем пустое место
        printf("         ");
    }

    // Выводим дату и имя файла
    printf("%s %s\n", mod_time, filename);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <файл1> <файл2> ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; i++) {
        print_file_info(argv[i]);
    }

    return EXIT_SUCCESS;
}