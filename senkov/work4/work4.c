#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEN 1024

typedef struct ListNode {
    char * data;
    struct ListNode *next;
} ListNode;

void add_node(ListNode ** list, char * new_str) {
    if (new_str == NULL) {
        fprintf(stderr, "Попытка добавить NULL строку\n");
        exit(EXIT_FAILURE);
    }

    ListNode * new_node = malloc(sizeof(ListNode));
    if (new_node == NULL) {
        perror("Ошибка выделения памяти для узла");
        exit(EXIT_FAILURE);
    }

    size_t len = strlen(new_str);
    if (len >= MAX_LEN) {
        fprintf(stderr, "Строка слишком длинная\n");
        free(new_node);
        exit(EXIT_FAILURE);
    }

    new_node->data = malloc(len + 1);
    if (new_node->data == NULL) {
        perror("Ошибка выделения памяти для строки");
        free(new_node);
        exit(EXIT_FAILURE);
    }

    strcpy(new_node->data, new_str);
    new_node->next = NULL;

    if (*list == NULL) {
        *list = new_node;
    } else {
        ListNode * curr = *list;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = new_node;
    }
}

char * read_line() {
    static char buffer[MAX_LEN];
    if (fgets(buffer, MAX_LEN, stdin) == NULL) {
        return NULL; // EOF или ошибка
    }

    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }

    return buffer;
}

void print_list(ListNode* list) {
    ListNode* current = list;
    int count = 1;
    printf("=== Все строки ===\n");
    while (current != NULL) {
        printf("%d: %s\n", count, current->data);
        current = current->next;
        count++;
    }
}

void free_memory(ListNode* list) {
    ListNode * curr = list;
    while (curr != NULL) {
        ListNode* next = curr->next;
        free(curr->data);
        free(curr);
        curr = next;
    }
}

// Функция для удаления пробелов в начале и конце
char * trim(char *str) {
    char *end;
    while (*str == ' ' || *str == '\t') str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t')) end--;
    *(end + 1) = '\0';
    return str;
}

int main(void) {
    ListNode* list = NULL;
    char * line;

    printf("Вводите строки (для завершения введите '.' в начале строки):\n");

    while (1) {
        line = read_line();
        if (line == NULL) break; // EOF

        char *trimmed = trim(line);
        if (trimmed[0] == '.') {
            break;
        }

        add_node(&list, line);
    }

    print_list(list);
    free_memory(list);

    return 0;
}
