#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


int main(){
    int files[2];


    if (pipe(files) == -1){
        printf("error");
    }
    
    pid_t pid = fork();

    if (pid == -1){
        printf("error");
    }

    if (pid == 0){
        close(files[1]);

        char c;

        while(read(files[0], &c, 1) > 0){
            c = toupper(c);
            write(STDOUT_FILENO, &c, 1);
        }
        write(STDOUT_FILENO, "\n", 1);
        close(files[0]);
    } else{
        close(files[0]);
        const char *str = "Hi WoRlD!";
        write(files[1], str, strlen(str));
        close(files[1]);
    }

    return 0;
}
