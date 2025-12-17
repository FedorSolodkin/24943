#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 200

typedef struct node
{
    char *str;
    struct node *next;
} StrNode;

typedef struct lst
{
    StrNode *head;
    StrNode *tail;
} StrList;

// Not used, but useful
StrList* list_init()
{
    StrList *mem = malloc(sizeof(StrList));
    mem->head = NULL;
    mem->tail = NULL;
    return mem;
}

void list_append(StrList *ls, char *st)
{
    StrNode *new = malloc(sizeof(StrNode));
    if (new == NULL)
    {
        perror("Couldn't allocate memory.");
        return;
    }
    new->str = st;
    new->next = NULL;

    if (ls->tail == NULL)
    {
        ls->head = new;
        ls->tail = new;
    }
    else
    {
        ls->tail->next = new;
        ls->tail = new;
    }
}

void list_destroy(StrList *ls)
{
    StrNode *it = ls->head;
    while (it != NULL)
    {
        StrNode *next = it->next;
        free(it->str);
        free(it);
        it = next;
    }
    ls->head = NULL;
    ls->tail = NULL;
}


char buffer[BUFFER_SIZE];

int main(int argc, char const *argv[])
{
    StrList list = {NULL, NULL};
    while (1)
    {
        fgets(buffer, BUFFER_SIZE, stdin);
        if (buffer[0] == '.') break;
        int length = strlen(buffer);
        char *input = malloc(length+1);
        strlcpy(input, buffer, BUFFER_SIZE);
        list_append(&list, input);
    }

    for (StrNode *it = list.head; it != NULL; it = it->next)
    {
        printf("%s\n", it->str);
    }
    list_destroy(&list);
    
    return 0;
}
