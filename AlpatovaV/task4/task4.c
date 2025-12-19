#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUF_SIZE 256

typedef struct ListString{
    char * str;
    struct ListString * next;
} ListString;

ListString* append_string(ListString * buffer, char * str){
    ListString *new_buffer = (ListString*)malloc(sizeof(ListString));

    new_buffer->str = (char*)malloc(strlen(str) + 1);
    strcpy(new_buffer->str, str);

    new_buffer->next = NULL;

    if (buffer == NULL){
        return new_buffer;

    }

    ListString *current = buffer;
    while (current->next != NULL) {
        current = current->next;
    }
    
    current->next = new_buffer;
    return buffer;

}  

void print_string(ListString * buffer){
    ListString * current = buffer;

    while (current != NULL){
        printf("%s", current->str);
        if (current->str[strlen(current->str) - 1] != '\n') {
            printf("\n");
        }
        current = current->next;
    }


}

void free_string(ListString * buffer) {
    ListString * current = buffer;
    while (current != NULL) {
        ListString * next = current->next;
        free(current->str);
        free(current);
        current = next;
    }
}


int main(){
    char buffer[BUF_SIZE];
    ListString *head = NULL;

    while (fgets(buffer, BUF_SIZE, stdin) != NULL){
        if (buffer[0] == '.'){
            break;
        }

        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        head = append_string(head, buffer);
    }

    print_string(head);
    free_string(head);
    return 0;
}
