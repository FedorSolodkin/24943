#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

int main(void){
    time_t now;
    struct tm *sp;
    
    // Получаем текущее время
    (void)time(&now);

    // Устанавливаем временную зону Калифорнии (PST) БЕЗ летнего времени
    putenv("TZ=PST8");
    tzset();

    // Преобразуем время
    sp = localtime(&now);
    
    // Принудительно устанавливаем СТАНДАРТНОЕ время (не летнее)
    sp->tm_isdst = 0;
    
    printf("California Time (PST): ");
    printf("%d/%d/%d %02d:%02d %s\n",
           sp->tm_mon + 1,      // месяц (1-12)
           sp->tm_mday,         // день месяца
           sp->tm_year + 1900,  // год
           sp->tm_hour,         // часы
           sp->tm_min,          // минуты
           tzname[0]);          // принудительно используем стандартное время (PST)
    
    return 0;
}
