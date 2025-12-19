#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

int count = 0;

void handleSIGINT() {
    signal(SIGINT, handleSIGINT);
    
    printf("\a");
    fflush(stdout);
    count++;
}

void handleSIGQUIT() {
    printf("\nThe signal sounded %d times.\n", count);
    exit(0);
}

int main() {
    signal(SIGINT, handleSIGINT);
    signal(SIGQUIT, handleSIGQUIT);

    printf("Program is running. Press Ctrl+C multiple times, then Ctrl+\\ to exit\n");
    
    while (1);
}
