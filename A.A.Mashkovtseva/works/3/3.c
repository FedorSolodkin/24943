#include <stdio.h>
#include <unistd.h>

int main(){
    printf("Real uid: %d\n", getuid());
    printf("Effective uid: %d\n", geteuid());
    FILE *file;
    if (file = fopen("file.txt", "r")) {
        fclose(file);
    } else {
        perror("Sorry, you can't open file.txt for reading");
    }

    setuid(geteuid());

    printf("Real uid: %d\n", getuid());
    printf("Effective uid: %d\n", geteuid());

    if (file = fopen("file.txt", "r")) {
        fclose(file);
    } else {
        perror("Sorry, you can't open file.txt for reading");
    }

    return 0;
}