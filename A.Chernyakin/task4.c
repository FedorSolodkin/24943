#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 256

typedef struct Node_s {
    char *string;
    struct Node_s *next;
} Node;

Node *head = NULL; // Убираем глобальные переменные, инициализируем в main
Node *tail = NULL;

// Функция для добавления строки в конец списка
void push(char *string) {
    unsigned long len = strlen(string) + 1;
    char *copyPtr = malloc(len); // Используем malloc, как в задании
    if (!copyPtr) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    strcpy(copyPtr, string);

    Node *newNode = malloc(sizeof(Node));
    if (!newNode) {
        free(copyPtr);
        exit(1);
    }

    newNode->string = copyPtr;
    newNode->next = NULL;

    if (head == NULL) {
        head = newNode;
        tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }
}

// Функция для вывода всех строк из списка
void printList() {
    Node *ptr = head;
    while (ptr != NULL) {
        printf("%s\n", ptr->string);
        ptr = ptr->next;
    }
}

// Освобождение памяти списка
void freeList() {
    Node *current = head;
    while (current != NULL) {
        Node *next = current->next;
        free(current->string);
        free(current);
        current = next;
    }
    head = tail = NULL;
}

int main() {
    char inputBuf[BUFFER_SIZE];

    printf("Enter '.' to print results.\n");

    while (1) {
        printf("Enter string: ");
        fflush(stdout);

        // Читаем строку с помощью fgets
        if (fgets(inputBuf, BUFFER_SIZE, stdin) == NULL) {
            printf("\nInput ended (EOF).\n");
            break;
        }

        // Удаляем символ новой строки, если он есть
        size_t len = strlen(inputBuf);
        if (len > 0 && inputBuf[len - 1] == '\n') {
            inputBuf[len - 1] = '\0';
            len--;
        }

        // Проверяем, является ли строка точкой (начинается с '.')
        if (len == 1 && inputBuf[0] == '.') {
            printf("Result:\n");
            printList();
            break;
        }

        // Проверка на пустую строку
        if (len == 0) {
            printf("Empty string not added.\n");
            continue;
        }

        // Проверка на допустимые символы (только буквы и пробелы)
    

        // Добавляем строку в список
        push(inputBuf);
    }

    freeList(); // Освобождаем память
    return 0;
}
