#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>

#define MAX_LINES 4096

// #define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
#define DEBUG(...)                    \
    {                                 \
        printf("========== DEBUG: "); \
        printf(__VA_ARGS__);          \
    }                                 
#else 
#define DEBUG(...) {}
#endif

typedef struct 
{
    size_t offset;
    size_t len;
} LineInfo;

size_t capacity = 10;
size_t line_cnt = 0;
LineInfo* table = NULL;
int fd_global;

bool is_user_managed = false;

void add_elem(const LineInfo elem)
{
    if (table == NULL)
    {
        table = (LineInfo*)malloc(capacity * sizeof(LineInfo));
        if (table == NULL)
        {
            perror("Initial malloc");
            exit(1);
        }
    }

    if (line_cnt >= capacity)
    {
        LineInfo* tmp = (LineInfo*)realloc(table, (capacity * 2) * sizeof(LineInfo));
        if (tmp == NULL)
        {
            perror("realloc");
            exit(1);
        }
        table = tmp;
        capacity *= 2;
    }

    *(table + line_cnt++) = elem;
}

void build_lookup_table(const int fd)
{
    char c;
    ssize_t bytes_read;

    LineInfo current_line = { .offset = 0, .len = 0 };

    while ((bytes_read = read(fd, &c, 1)) > 0)
    {
        current_line.len++;
        if (c == '\n')
        {
            add_elem(current_line);

            current_line.offset = lseek(fd, 0, SEEK_CUR); 
            current_line.len = 0;
        }
    }

    if (bytes_read == 0 && current_line.len > 0)
    {
        add_elem(current_line);
    }

    if (bytes_read == -1)
    {
        perror("read");
        exit(1);
    }
}

void timeout_handler(int s)
{
    printf("\nTime is up. File contents:\n\n");

    lseek(fd_global, 0, SEEK_SET);
    char buf[256];
    ssize_t n;
    while ((n = read(fd_global, buf, sizeof(buf))) > 0)
    {
        write(STDOUT_FILENO, buf, n);
    }

    close(fd_global);
    free(table);
    exit(0);
}

void print_table(LineInfo* table)
{
    printf("===== lines table =====\n");
    for (size_t i = 0; i < line_cnt; i++)
    {
        printf("line: %zu\n", i+1);
        printf("offset: %zu\n", table[i].offset);
        printf("length: %zu\n\n", table[i].len);
    }
    printf("=======================\n");
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Program expects a file path as an argument.\n");
        return 1;
    }

    char* filepath = argv[1];
    int fd = open(filepath, O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        return 1;
    }

    build_lookup_table(fd);

    fd_global = fd;

    signal(SIGALRM, timeout_handler);

    while(1)
    {
        if (is_user_managed)
        {
            printf("Enter line number (0 to exit). Enter \"t\" to watch lines table.: ");
        }

        if (!is_user_managed)
        {
            printf("Enter line number (0 to exit). Enter \"t\" to watch lines table. You have 5 seconds: ");
            alarm(5);
        }
        
        char* input_buf = (char*)malloc(256);
        if (fgets(input_buf, 256, stdin) == NULL)
        {
            perror("fgets");
            free(input_buf);
            break;
        }

        if (input_buf[0] == 't')
        {
            print_table(table);
            continue;
        }

DEBUG("полученная строка до валидации: %s\n", input_buf);
        
        char* ptr = input_buf;
        size_t line_num = strtod(input_buf, &ptr);

DEBUG("полученное число до валидации: %zu\n\n", line_num);
DEBUG("ptr: %c, input_buf: %c\n\n", *ptr == '\n' ? '\\n' : *ptr, *input_buf);

        if (ptr == input_buf)
        {
            fprintf(stderr, "Enter a number of line in file. Enter \"t\" to watch lines table.\n");
            free(input_buf);
            continue;
        }

        if (line_num == 0)
        {
            free(input_buf);
            printf("Exiting.\n");
            break;
        }

        if (line_num < 0 || line_num > line_cnt)
        {
            is_user_managed = true;
            alarm(0);
            free(input_buf);
            printf("Line number must be between 1 and %zu.\n Enter \"t\" to watch lines table.\n", line_cnt);
            continue;
        }

        if (!is_user_managed)
        {
            is_user_managed = true;
            alarm(0);
        }

        LineInfo info = table[line_num - 1];

        if (lseek(fd, info.offset, SEEK_SET) == -1)
        {
            free(input_buf);
            perror("lseek");
            continue;
        }

        char* line_content = (char*)malloc(info.len + 1);
        if (line_content == NULL)
        {
            free(input_buf);
            perror("malloc");
            continue;
        }
        ssize_t num_read = read(fd, line_content, info.len);
        if (num_read == -1)
        {
            free(input_buf);
            perror("read line");
            free(line_content);
            continue;
        }
        
        line_content[num_read] = '\0';

        printf("Line %ld: %s", line_num, line_content);
        
        free(input_buf);
        free(line_content);
    }

    close(fd);

    free(table);
    
    return 0;
}
