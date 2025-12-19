#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    char *s;
    struct Node *next;
} Node;

void print_escaped(const char* s){
    for (int i = 0; s[i]; i++){
        if (s[i] == 27 && s[i+1] == '['){
            i += 2;
            if (s[i] == 'A') printf("[UP]");
            else if (s[i] == 'B') printf("[DOWN]"); 
            else if (s[i] == 'C') printf("[RIGHT]");
            else if (s[i] == 'D') printf("[LEFT]");
            else if (s[i] == 'H') printf("[HOME]");
            else if (s[i] == 'F') printf("[END]");
            else if (s[i] == '1' && s[i+1] == '~') { printf("[HOME]"); i++; }
            else if (s[i] == '3' && s[i+1] == '~') { printf("[DEL]"); i++; }
            else if (s[i] == '4' && s[i+1] == '~') { printf("[END]"); i++; }
            else if (s[i] == '5' && s[i+1] == '~') { printf("[PGUP]"); i++; }
            else if (s[i] == '6' && s[i+1] == '~') { printf("[PGDN]"); i++; }
            else if (s[i] == '7' && s[i+1] == '~') { printf("[HOME]"); i++; }
            else if (s[i] == '8' && s[i+1] == '~') { printf("[END]"); i++; }
            else { printf("\\x1B[%c", s[i]); }
        } else{
            putchar(s[i]);
        }
    }
}

int main(void) {
    enum { MAX = 4096 };
    char buf[MAX];

    Node *head = NULL, *tail = NULL;

    puts("Введите строки. Точка в начале строки завершает ввод.");
    while (fgets(buf, sizeof(buf), stdin)){
        if (buf[0] == '.') break;

        size_t len = strlen(buf);
        if (len && buf[len-1] == '\n') buf[--len] = '\0';

        char *copy = (char *)malloc(len + 1);
        if (!copy) { perror("malloc"); return 1; }
        memcpy(copy, buf, len + 1);

        Node *node = (Node *)malloc(sizeof(Node));
        if (!node) { perror("malloc"); free(copy); return 1; }
        node->s = copy;
        node->next = NULL;

        if (!head) head = tail = node;
        else { tail->next = node; tail = node; }
    }

    puts("\nСодержимое списка:");
    for (Node *p = head; p; p = p->next){
        print_escaped(p->s);
        putchar('\n');
    }

    while (head){
        Node *n = head->next;
        free(head->s);
        free(head);
        head = n;
    }
    return 0;
}