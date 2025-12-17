#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Node {
    char* str;
    struct Node* next;
};

void print_escaped(const char* s) {
    for (int i = 0; s[i]; i++) {
        if (s[i] == 27 && s[i + 1] == '[') {
            i += 2;
            if (s[i] == 'A') printf("up");
	    else if (s[i] == 'B') printf("down");
	    else if (s[i] == 'C') printf("right");
	    else if (s[i] == 'D') printf("left");
	    else if (s[i] == 'H') printf("home");
	    else if (s[i] == 'F') printf("end");
	    else if (s[i] == '1' && s[i + 1] == '~') { printf("home"); i++; }
	    else if (s[i] == '3' && s[i + 1] == '~') { printf("del"); i++; }
	    else if (s[i] == '4' && s[i + 1] == '~') { printf("and"); i++; }
	    else if (s[i] == '5' && s[i + 1] == '~') { printf("pgup"); i++; }
	    else if (s[i] == '6' && s[i + 1] == '~') { printf("pgdn"); i++; }
	    else if (s[i] == '7' && s[i + 1] == '~') { printf("home"); i++; }
	    else if (s[i] == '8' && s[i + 1] == '~') { printf("and"); i++; }
            else { printf("\\x1B[%c", s[i]); }
        }
        else {
            putchar(s[i]);
        }
    }
}

int main() {
    struct Node* head = NULL;
    struct Node* tail = NULL;

    char buffer[1024];

    while (1) {
        printf("Enter str (or '.' for end) : ");
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }

        if (buffer[0] == '.') {
            break;
        }
        
        buffer[strcspn(buffer, "\n")] = '\0';

        size_t len = strlen(buffer);

        char* new_str = malloc(len + 1);
        if (new_str == NULL) {
            fprintf(stderr, "Error memory\n");
            exit(1);
        }
        strcpy(new_str, buffer);

        struct Node* new_node = malloc(sizeof(struct Node));
        if (new_node == NULL) {
            fprintf(stderr, "Error memory\n");
            exit(1);
        }
        new_node->str = new_str;
        new_node->next = NULL;

        if (head == NULL) {
            head = new_node;
            tail = new_node;
        }
        else {
            tail->next = new_node;
            tail = new_node;
        }
    }


    printf("\nSave str:\n");
    struct Node* current = head;
    while (current != NULL) {
	print_escaped(current->str);
        printf("\n");
        current = current->next;
    }

    return 0;
}
