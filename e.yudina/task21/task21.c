#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

int count = 0;

void handleSIGINT() {
    printf(" BEEP %d\n", ++count);
    fflush(NULL);
    signal(SIGINT, handleSIGINT);
}

void handleSIGQUIT() {
    printf("\nThe signal sounded %d times.", count);
    exit(0);
}

int main() {
    signal(SIGINT, handleSIGINT);
    signal(SIGQUIT, handleSIGQUIT);

    while (1);
}
