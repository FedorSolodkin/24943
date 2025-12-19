#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main(void){
    if (setenv("TZ", "America/Los_Angeles", 1) != 0){
        putenv("TZ=America/Los_Angeles");
    }
    tzset();

    time_t now = time(NULL);
    struct tm *pt = localtime(&now);
    if (!pt) return 1;

    char buf[64];
    pt->tm_hour -=1;
    if (strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", pt) == 0) return 1;

    printf("California time: %s PST\n", buf);
    return 0;
}
