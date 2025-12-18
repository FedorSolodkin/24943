#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    char command[256];

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    sprintf(command, "grep -c '^$' %s", argv[1]);
    system(command);

    return 0;
}
