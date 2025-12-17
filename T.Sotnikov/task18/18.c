#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <libgen.h> // для basename

// Функция для обработки одного файла
void process_file(const char *path) {
    struct stat st;

    // Получаем информацию о файле. Если ошибка (например, файла нет) - выводим сообщение
    if (stat(path, &st) == -1) {
        perror(path);
        return;
    }

    // 1. Тип файла (d - каталог, - - файл, ? - прочее)
    char type = '?';
    if (S_ISDIR(st.st_mode)) type = 'd';
    else if (S_ISREG(st.st_mode)) type = '-';

    // 2. Права доступа (rwx для владельца, группы и остальных)
    char perms[10];
    const char *modes = "rwxrwxrwx";
    // Проходим по 9 битам прав (от старшего к младшему)
    for (int i = 0; i < 9; i++) {
        // Сдвиг битов для проверки конкретного флага
        perms[i] = (st.st_mode & (1 << (8 - i))) ? modes[i] : '-';
    }
    perms[9] = '\0'; // Завершаем строку

    // 3. Количество жестких ссылок
    long links = (long)st.st_nlink;

    // 4. Имя владельца и группы
    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);
    const char *user_name = pw ? pw->pw_name : "unknown";
    const char *group_name = gr ? gr->gr_name : "unknown";

    // 5. Размер (только для обычных файлов)
    char size_str[20] = "";
    if (S_ISREG(st.st_mode)) {
        sprintf(size_str, "%ld", (long)st.st_size);
    }

    // 6. Дата модификации
    char date_str[20];
    struct tm *tm_info = localtime(&st.st_mtime);
    strftime(date_str, sizeof(date_str), "%b %d %H:%M", tm_info);

    // 7. Имя файла (без пути)
    // Используем простую логику поиска последнего слэша, чтобы не менять входную строку
    const char *filename = strrchr(path, '/');
    if (filename) filename++; // Пропускаем слэш
    else filename = path;     // Если слэша нет, берем как есть

    // Вывод таблицей (числа в printf задают ширину поля)
    printf("%c%s %3ld %-8s %-8s %8s %s %s\n",
           type, perms, links, user_name, group_name, size_str, date_str, filename);
}

int main(int argc, char *argv[]) {
    // Базовая валидация: проверим, передали ли аргументы
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <файл1> [файл2 ...]\n", argv[0]);
        return 1;
    }

    // Обрабатываем каждый аргумент по очереди
    for (int i = 1; i < argc; i++) {
        process_file(argv[i]);
    }

    return 0;
}