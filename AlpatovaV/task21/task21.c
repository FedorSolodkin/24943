#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

int count = 0;

void handleSIGINT(int sig) {
    signal(SIGINT, handleSIGINT);

    printf("signal\n");
    fflush(NULL);
    count++;
}

void handleSIGQUIT(int sig) {
    printf("\nThe signal sounded %d times.", count);
    exit(0);
}

int main(){
    signal(SIGINT, &handleSIGINT);
    signal(SIGQUIT, &handleSIGQUIT);


    printf("Press Ctrl+C multiple times, then Ctrl+\\ to exit\n");
    while (1);

}
