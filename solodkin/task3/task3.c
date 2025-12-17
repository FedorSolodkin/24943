#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    printf("=== Before setuid ===\n");
    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());
    
    FILE *file = fopen("secret.txt", "r");
    if (file) {
        printf("File opened successfully\n");
        fclose(file);
    } else {
        perror("Error opening file");
    }
    
    printf("\n=== After setuid ===\n");
    setuid(getuid());
    
    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());
    
    file = fopen("secret.txt", "r");
    if (file) {
        printf("File opened successfully\n");
        fclose(file);
    } else {
        perror("Error opening file");
    }
    
    return 0;
}
