#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int beep_counter = 0;

void beep_function(int num) {
    char alert = 7;
    write(1, &alert, 1);
    beep_counter = beep_counter + 1;
}

void exit_function(int num) {
    fprintf(stdout, "\nTotal alerts: %d\n", beep_counter);
    exit(1);
}

int start_program() {
    signal(2, beep_function);
    signal(3, exit_function);
    
    while (1) {
        sleep(1);
    }
}

int main() {
    start_program();
    return 1;
}