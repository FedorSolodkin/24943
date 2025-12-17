#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024

typedef struct Node {
    char *data;
    struct Node *next;
} Node;

Node* create_node(const char *str) {
    Node *new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "Ошибка выделения памяти под новый узел\n");
        return NULL;
    }
    size_t len = strlen(str);
    new_node->data = (char*)malloc(len + 1);
    if (new_node->data == NULL) {
        fprintf(stderr, "Ошибка выделения памяти для одной строки\n");
        free(new_node);
        return NULL;
    }
    
    strcpy(new_node->data, str);
    new_node->next = NULL;
    
    return new_node;
}

void append_node(Node **head, Node **tail, const char *str) {
    Node *new_node = create_node(str);
    if (new_node == NULL) return;
    
    if (*head == NULL) {
        *head = new_node;
        *tail = new_node;
    } else {
        (*tail)->next = new_node;
        *tail = new_node;
    }
}

void print_list(Node *head) {
    Node *current = head;
    printf("\nСписок введенных строк:\n");    
    while (current != NULL) {
        printf("%s", current->data);
        current = current->next;
    }
}

void free_list(Node *head) {
    Node *current = head;
    Node *next;
    
    while (current != NULL) {
        next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
}

int main() {
    char buffer[MAX_LINE_LENGTH];
    Node *head = NULL;
    Node *tail = NULL;
    
    printf("Введите строки (для завершения введите '.' в начале строки):\n");
    
    while (1) {
        if (fgets(buffer, MAX_LINE_LENGTH, stdin) == NULL) break;
        if (buffer[0] == '.') break;
        append_node(&head, &tail, buffer);
    }
    
    print_list(head);
    free_list(head);
    
    return 0;
}
