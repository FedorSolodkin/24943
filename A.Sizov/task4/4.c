#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 1024
#define MAXSTR 1000

int main() {
    char buffer[MAXLINE];
    char *lines[MAXSTR];
    int count = 0;

    while (fgets(buffer, MAXLINE, stdin)) {
        if (buffer[0] == '.') break;

        size_t len = strlen(buffer);
        if (buffer[len - 1] == '\n') buffer[len - 1] = '\0';

        lines[count] = malloc(len);
        strcpy(lines[count], buffer);
        count++;
    }

    for (int i = 0; i < count; i++) {
        printf("%s\n", lines[i]);
        free(lines[i]);
    }

    return 0;
}
