#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STR_LEN 100

typedef struct
{
    char* data;
    struct Node* next;
} Node;

void push(Node** head, const char* data)
{
    if (head == NULL || data == NULL)
    {
        fprintf(stderr, "NULL pointer passed to push\n");
        exit(EXIT_FAILURE);
    }

    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    new_node->data = strdup(data);
    if (!new_node->data)
    {
        free(new_node);
        perror("strdup");
        exit(EXIT_FAILURE);
    }
    
    new_node->next = NULL;

    if (*head == NULL)
    {
        *head = new_node;
        return;
    }

    Node* curr_node = *head;
    while (curr_node->next != NULL)
        curr_node = (Node*)curr_node->next;

    curr_node->next = (struct Node*)new_node;
}

// char* pop(Node** to_push)
// {
//     return NULL;
// }

void print_nodes(Node** head)
{
    if (head == NULL)
    {
        fprintf(stderr, "A NULL pointer was passed.\n");
        exit(EXIT_FAILURE);
    }

    if (*head == NULL)
        return;

    Node* curr_node = *head;
    while (curr_node != NULL)
    {
        printf("%s", curr_node->data);
        curr_node = (Node*)curr_node->next;
    }

    printf("\n");
}

void free_nodes(Node** head)
{
    if (head == NULL)
    {
        fprintf(stderr, "A NULL pointer was passed.\n");
        exit(EXIT_FAILURE);
    }

    Node* curr_node = *head;
    while (curr_node->next != NULL)
    {
        Node* tmp_node = (Node*)curr_node->next;
        free(curr_node->data);
        free(curr_node);
        curr_node = tmp_node;
    }
    *head = NULL;
}

int main(void)
{
    Node* nodes = NULL;

    printf("===== Enter the lines (dot to end): =====\n");

    char* str = (char*)malloc(MAX_STR_LEN);
    while (1)
    {
        fgets(str, MAX_STR_LEN, stdin);

        if (str[0] == '.')
            break;

        push(&nodes, str);
    }

    printf("\n================ Output: ================\n");

    print_nodes(&nodes);

    free_nodes(&nodes);

    return 0;
}
