#include <unistd.h>
#include <sys/termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdarg.h>

#define LINE_LENGTH 40

static struct termios orig_termios;

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
        perror("tcsetattr disableRawMode");
    }
}

void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        exit(1);
    }
    
    if (atexit(disableRawMode) != 0) {
        perror("atexit");
        exit(1);
    }

    struct termios raw = orig_termios;
    
    raw.c_lflag &= ~(ECHO | ICANON);
    
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr enableRawMode");
        exit(1);
    }
}

void sigtstp_handler(int sig) {
    disableRawMode();
    signal(SIGTSTP, SIG_DFL);
    raise(SIGTSTP);
}

void sigcont_handler(int sig) {
    signal(SIGTSTP, sigtstp_handler);
    enableRawMode();
}

void termination_handler(int sig) {
    disableRawMode();
    _exit(0);
}

void setup_signal_handlers() {
    struct sigaction sa;
    
    signal(SIGTSTP, sigtstp_handler);
    signal(SIGCONT, sigcont_handler);
    
    sa.sa_handler = termination_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
}

void safe_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fflush(stdout);
}

int get_current_line_display_length(const char* line, int cursor_pos) {
    return cursor_pos % (LINE_LENGTH + 1);
}

int find_word_start(const char* line, int cursor_pos) {
    for (int i = cursor_pos - 1; i >= 0; i--) {
        if (line[i] == ' ') {
            return i + 1;
        } else if (i == 0) {
            return 0;
        }
    }
    return cursor_pos;
}

void redisplay_line(const char* line, int len) {
    safe_printf("\r\33[K"); // Clear current line
    
    for (int i = 0; i < len; i++) {
        if (i > 0 && i % LINE_LENGTH == 0) {
            safe_printf("\n");
        }
        putchar(line[i]);
    }
    fflush(stdout);
}

int main() {
    if (!isatty(STDIN_FILENO)) {
        fprintf(stderr, "Error: stdin is not a terminal\n");
        return 1;
    }
    
    setup_signal_handlers();
    enableRawMode();

    char c;
    static char line[LINE_LENGTH * 4 + 1] = {0};
    int len = 0;

    while (read(STDIN_FILENO, &c, 1) == 1) {
        if (iscntrl(c) || !isprint(c)) {
            switch (c) {
                case 0x7F: // Backspace
                    {
                        if (len > 0) {
                            len--;
                            line[len] = 0;
                            redisplay_line(line, len);
                        } else {
                            safe_printf("\a");
                        }
                        break;
                    }

                case 0x15: // Ctrl+U - erase line
                    {
                        if (len > 0) {
                            len = 0;
                            line[0] = 0;
                            redisplay_line(line, len);
                        }
                        break;
                    }

                case 0x17: // Ctrl+W - erase word
                    {
                        if (len > 0) {
                            int word_start = len;
                            int in_word = 0;
                            
                            for (int i = len - 1; i >= 0; i--) {
                                if (line[i] != ' ') {
                                    in_word = 1;
                                    word_start = i;
                                } else if (in_word && line[i] == ' ') {
                                    break;
                                }
                            }
                            
                            // Remove trailing spaces
                            while (word_start < len && line[word_start] == ' ') {
                                word_start++;
                            }
                            
                            len = word_start;
                            line[len] = 0;
                            redisplay_line(line, len);
                        }
                        break;
                    }

                case 0x04: // Ctrl+D
                    {
                        if (len == 0) {
                            safe_printf("\n");
                            disableRawMode();
                            exit(0);
                        }
                        break;
                    }

                default:
                    {
                        safe_printf("\a");
                        break;
                    }
            }
        } else {
            // Printable character
            if (len < LINE_LENGTH * 4 - 1) {
                int current_display_len = get_current_line_display_length(line, len);
                
                // Check if we need to wrap to new line
                if (current_display_len >= LINE_LENGTH) {
                    // Find where the current word starts
                    int word_start = find_word_start(line, len);
                    int word_length = len - word_start;
                    
                    // If the word fits on a new line, wrap it
                    if (word_length < LINE_LENGTH) {
                        // Just continue typing - the redisplay will handle wrapping
                        line[len++] = c;
                        line[len] = 0;
                        redisplay_line(line, len);
                    } else {
                        // Word is too long, just add character
                        line[len++] = c;
                        line[len] = 0;
                        putchar(c);
                    }
                } else {
                    // Normal case - just add character
                    line[len++] = c;
                    line[len] = 0;
                    
                    // Check if we need to wrap after adding this character
                    int new_display_len = get_current_line_display_length(line, len);
                    if (new_display_len == 0 && len > 1) {
                        // We wrapped to a new line, redisplay to show it properly
                        redisplay_line(line, len);
                    } else {
                        putchar(c);
                    }
                }
            } else {
                safe_printf("\a");
            }
        }

        fflush(stdout);
    }

    if (errno != EINTR) {
        perror("read");
    }

    disableRawMode();
    return 0;
}
