#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

void print_uids(const char* when) {
    printf("rUID: %d, eUID: %d\n", (int)getuid(), (int)geteuid());
}

void try_open_file(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        perror("fopen");
    }
    else {
        printf("File '%s' opened and closed!\n", filename);
        fclose(f);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];

    print_uids("Before setuid");
    try_open_file(filename);
    
    setuid(geteuid());

    print_uids("After setuid");
    try_open_file(filename);

    return 0;
}
