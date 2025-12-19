#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

extern char *tzname[];
int main(void)
{
    time_t now;
    
    time(&now);
    struct tm *utc = gmtime(&now);
    printf("UTC время: %d/%d/%02d %d:%02d\n",
        utc->tm_mon + 1, utc->tm_mday,
        utc->tm_year + 1900, utc->tm_hour,
        utc->tm_min);

    setenv("TZ", "UTC+8", 1);
    tzset();
    time(&now);
    struct tm *sp = localtime(&now);
    printf("UTC+8: %d/%d/%02d %d:%02d\n",
        sp->tm_mon + 1, sp->tm_mday,
        sp->tm_year + 1900, sp->tm_hour,
        sp->tm_min);

    time(&now);
    sp = gmtime(&now);
    int pst_hour = (sp->tm_hour - 8 + 24) % 24;  // Корректная арифметика часов
    printf("Прямое вычитание: %d/%d/%02d %d:%02d\n",
        sp->tm_mon + 1, sp->tm_mday,
        sp->tm_year + 1900, pst_hour,
        sp->tm_min);

    exit(0);
}