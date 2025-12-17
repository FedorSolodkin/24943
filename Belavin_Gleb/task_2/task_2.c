#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

int main() {
    time_t now;
    struct tm *pst_time;
    
    time(&now);

    setenv("TZ", "US/Pacific", 1);
    tzset();

    pst_time = localtime(&now);
    if (pst_time == NULL) {
        perror("localtime");
        return 1;
    }
    
    char buffer[100];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S %Z", pst_time);
    printf("Текущее время в Калифорнии: %s\n", buffer);
    
    return 0;
}