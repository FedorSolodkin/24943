#include <termios.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    struct termios original_settings, raw_mode_settings;
    
    tcgetattr(STDIN_FILENO, &original_settings);
    raw_mode_settings = original_settings;

    raw_mode_settings.c_lflag &= ~(ICANON | ECHO | ISIG);
    raw_mode_settings.c_cc[VMIN] = 1;
    raw_mode_settings.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &raw_mode_settings);

    char erase_char = raw_mode_settings.c_cc[VERASE];
    char kill_char = raw_mode_settings.c_cc[VKILL];

    char input_buffer[80];
    int buffer_length = 0;
    int cursor_column = 0;

    char current_char;
    while (read(STDIN_FILENO, &current_char, 1) == 1) {
        if (current_char == 4) {
            if (buffer_length == 0) break;
            write(STDOUT_FILENO, "\a", 1);
        } 
        else if (current_char == '\n' || current_char == '\r') {
            write(STDOUT_FILENO, "\n", 1);
            buffer_length = 0;
            cursor_column = 0;
        } 
        else if (current_char == erase_char) {
            if (buffer_length > 0) {
                write(STDOUT_FILENO, "\b \b", 3);
                buffer_length--;
                cursor_column--;
            } else {
                write(STDOUT_FILENO, "\a", 1);
            }
        } 
        else if (current_char == kill_char) {
            while (buffer_length > 0) {
                write(STDOUT_FILENO, "\b \b", 3);
                buffer_length--;
            }
            cursor_column = 0;
        } 
        else if (current_char == 23) {
            if (buffer_length == 0) {
                write(STDOUT_FILENO, "\a", 1);
            } else {
                while (buffer_length > 0 && input_buffer[buffer_length-1] == ' ') {
                    write(STDOUT_FILENO, "\b \b", 3);
                    buffer_length--; 
                    cursor_column--;
                }
                while (buffer_length > 0 && input_buffer[buffer_length-1] != ' ') {
                    write(STDOUT_FILENO, "\b \b", 3);
                    buffer_length--; 
                    cursor_column--;
                }
            }
        }
        else if (current_char == 27) {
            write(STDOUT_FILENO, "\a", 1);
            char escape_char;
            while (read(STDIN_FILENO, &escape_char, 1) == 1) {
                if ((escape_char >= 'A' && escape_char <= 'Z') || escape_char == '~')
                    break;
            }
        }
        else if (current_char >= 32 && current_char < 127) {
            if (buffer_length < 79) {
                input_buffer[buffer_length++] = current_char;
                write(STDOUT_FILENO, &current_char, 1);
                cursor_column++;

                if (cursor_column > 40) {
                    int word_start = buffer_length - 1;
                    while (word_start > 0 && input_buffer[word_start-1] != ' ') 
                        word_start--;
                    
                    int word_length = buffer_length - word_start;

                    if (word_start > 0 && word_length <= 40) {
                        int position;
                        for (position = 0; position < word_length; position++)
                            write(STDOUT_FILENO, "\b \b", 3);

                        write(STDOUT_FILENO, "\n", 1);
                        
                        for (position = 0; position < word_length; position++)
                            input_buffer[position] = input_buffer[word_start + position];

                        buffer_length = word_length;
                        cursor_column = word_length;
                        write(STDOUT_FILENO, input_buffer, word_length);
                    }
                }
            }
        } 
        else {
            write(STDOUT_FILENO, "\a", 1);
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &original_settings);
    return 0;
}