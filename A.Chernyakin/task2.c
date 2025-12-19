#include <stdio.h>
#include <time.h>

int main(void) {
    time_t now;
    struct tm tm_pst;

    time(&now);

    now -= 8 * 3600;

    if (gmtime_r(&now, &tm_pst) == NULL) {
        perror("gmtime_r");
        return 1;
    }

    printf("%04d-%02d-%02d %02d:%02d:%02d PST (UTC-8)\n",
           tm_pst.tm_year + 1900,
           tm_pst.tm_mon + 1,
           tm_pst.tm_mday,
           tm_pst.tm_hour,
           tm_pst.tm_min,
           tm_pst.tm_sec);

    return 0;
}
