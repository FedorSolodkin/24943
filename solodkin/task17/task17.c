#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 40
#define MAX_TEXT_LENGTH 2000
#define BELL '\007'
#define ERASE_DEL 0x7F
#define ERASE_BS  0x08
#define KILL 0x15
#define CTRL_W 0x17
#define CTRL_D 0x04

struct termios original_termios;

void restore_terminal(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

void setup_terminal(void)
{
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &original_termios);
    atexit(restore_terminal);
    new_termios = original_termios;
    // Отключаем канонический режим и эхо
    new_termios.c_lflag &= ~(ICANON | ECHO); 
    new_termios.c_cc[VMIN] = 1;
    new_termios.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_termios);
}

struct editor
{
    char text[MAX_TEXT_LENGTH];
    int pos;
    int len;
};

// Функция возвращает длину следующего слова (до пробела или конца строки)
int get_next_word_len(const char *text, int start, int len) {
    int i = start;
    while (i < len && text[i] != ' ' && text[i] != '\t') {
        i++;
    }
    return i - start;
}

void redraw(struct editor *e)
{
    // Очистка экрана и перемещение в начало
    printf("\033[H\033[2J");
    printf("Input text (CTRL-D at line start to exit):");
    
    // Начальная позиция для вывода текста (строка 2, колонка 1)
    int start_row = 2;
    int current_col = 0;
    int current_row = 0; // Относительный номер строки (0, 1, 2...)

    // Координаты, куда нужно будет поставить курсор терминала
    int cursor_scr_row = start_row;
    int cursor_scr_col = 1;

    // Перемещаемся на позицию начала вывода
    printf("\033[%d;1H", start_row);

    int i = 0;
    while (i <= e->len) // <= чтобы обработать случай, когда курсор в самом конце
    {
        // Если текущий индекс совпадает с позицией курсора в редакторе, запоминаем экранные координаты
        if (i == e->pos) {
            cursor_scr_row = start_row + current_row;
            cursor_scr_col = current_col + 1;
        }

        if (i == e->len) break; // Конец текста

        char c = e->text[i];

        // Логика переноса слов
        if (c == ' ') {
            // Пробел просто печатаем, если он влезает. 
            // Если мы ровно на 40-м символе, пробел вызовет перенос.
            if (current_col >= MAX_LINE_LENGTH) {
                printf("\n");
                current_row++;
                current_col = 0;
            }
            putchar(c);
            current_col++;
            i++;
        }
        else {
            // Это начало слова. Проверяем, влезает ли оно целиком.
            int wlen = get_next_word_len(e->text, i, e->len);
            
            // Если слово не влезает в остаток строки, переносим на новую
            // (но только если это не начало строки, чтобы не зациклить бесконечно длинное слово)
            if (current_col + wlen > MAX_LINE_LENGTH && current_col > 0) {
                printf("\n");
                current_row++;
                current_col = 0;
            }

            // Печатаем слово (или его часть, если оно само по себе длиннее 40 символов)
            while (wlen > 0) {
                 // Если мы в процессе печати длинного слова уперлись в край
                if (current_col >= MAX_LINE_LENGTH) {
                    printf("\n");
                    current_row++;
                    current_col = 0;
                }
                
                // Проверка на позицию курсора внутри слова
                if (i == e->pos) {
                    cursor_scr_row = start_row + current_row;
                    cursor_scr_col = current_col + 1;
                }

                putchar(e->text[i]);
                i++;
                current_col++;
                wlen--;
            }
        }
    }

    // Ставим курсор на запомненное место
    printf("\033[%d;%dH", cursor_scr_row, cursor_scr_col);
    fflush(stdout);
}

void erase_word(struct editor *e)
{
    if (e->pos == 0)
    {
        putchar(BELL);
        fflush(stdout);
        return;
    }

    int end = e->pos;
    // Пропускаем пробелы справа налево
    while (end > 0 && e->text[end - 1] == ' ')
        end--;
    // Пропускаем символы слова справа налево
    while (end > 0 && e->text[end - 1] != ' ')
        end--;

    int n = e->pos - end;
    if (n > 0)
    {
        memmove(e->text + end, e->text + e->pos, e->len - e->pos + 1);
        e->len -= n;
        e->pos = end;
        redraw(e);
    }
}

int main(void)
{
    struct editor e = {.pos = 0, .len = 0};
    char c;

    setup_terminal();
    redraw(&e); // Первая отрисовка

    while (read(STDIN_FILENO, &c, 1) == 1)
    {
        if (c == CTRL_D)
        {
            if (e.pos == 0) {
                printf("\nExit.\n");
                break;
            } else {
                putchar(BELL);
                fflush(stdout);
            }
            continue;
        }

        // Обработка Backspace (может быть 0x7F или 0x08)
        if (c == ERASE_DEL || c == ERASE_BS)
        {
            if (e.pos > 0)
            {
                e.pos--;
                e.len--;
                memmove(e.text + e.pos, e.text + e.pos + 1, e.len - e.pos + 1);
                redraw(&e);
            }
            else
            {
                putchar(BELL);
                fflush(stdout);
            }
            continue;
        }

        if (c == KILL)
        {
            // KILL удаляет все от начала строки (буфера) до курсора
            if (e.pos > 0)
            {
                memmove(e.text, e.text + e.pos, e.len - e.pos + 1);
                e.len -= e.pos;
                e.pos = 0;
                redraw(&e);
            }
            else
            {
                putchar(BELL);
                fflush(stdout);
            }
            continue;
        }

        if (c == CTRL_W)
        {
            erase_word(&e);
            continue;
        }

        // Проверка на печатные символы (пробел - 32, тильда - 126)
        if (c < 32 || c > 126)
        {
            putchar(BELL);
            fflush(stdout);
            continue;
        }

        if (e.len >= MAX_TEXT_LENGTH - 1)
        {
            putchar(BELL);
            fflush(stdout);
            continue;
        }

        // Вставка символа
        if (e.pos < e.len)
        {
            memmove(e.text + e.pos + 1, e.text + e.pos, e.len - e.pos);
        }
        e.text[e.pos] = c;
        e.pos++;
        e.len++;
        e.text[e.len] = '\0';
        redraw(&e);
    }

    return 0;
}