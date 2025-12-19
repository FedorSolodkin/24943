#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>

#define BUFFER_SIZE 256

typedef struct Node_s {
    char *string;
    struct Node_s *next;
} Node;

Node *head, *tail;

void init() {
    head = malloc(sizeof(Node));
    head->string = NULL;
    head->next = NULL;
    tail = head;
}

void push(char *string) {
    unsigned long len = strlen(string) + 1;
    char *copyPtr = calloc(len, sizeof(char));
    strcpy(copyPtr, string);

    tail->string = copyPtr;
    tail->next = calloc(1, sizeof(Node));
    tail = tail->next;
}

void printList() {
    Node *ptr = head;
    while (ptr != NULL) {
        if (ptr->string) {
            printf("%s\n", ptr->string);
        }
        ptr = ptr->next;
    }
}

// Function to set terminal raw mode
void setRawMode(int enable) {
    static struct termios original;
    static int restored = 1;
    
    if (enable) {
        if (restored) {
            tcgetattr(STDIN_FILENO, &original);
            restored = 0;
        }
        
        struct termios raw = original;
        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    } else {
        if (!restored) {
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
            restored = 1;
        }
    }
}

// Function to check if character is allowed
int isAllowedChar(unsigned char c) {
    // Allow letters A-Z, a-z
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
        return 1;
    }
    // Allow space
    if (c == ' ') {
        return 1;
    }
    // Allow dot (only checked separately in main logic)
    if (c == '.') {
        return 1;
    }
    return 0;
}

// Function to read characters with filtering
int readFilteredLine(char *buffer, int size) {
    int pos = 0;
    unsigned char c;
    
    setRawMode(1);
    
    while (1) {
        if (read(STDIN_FILENO, &c, 1) != 1) {
            continue;
        }
        
        // Handle control characters and special cases
        if (c == '\n' || c == '\r') { // Enter - ASCII 10, 13
            printf("\n");
            break;
        } else if (c == 127 || c == 8) { // Backspace - ASCII 8, 127
            if (pos > 0) {
                pos--;
                printf("\b \b");
                fflush(stdout);
            }
        } else if (c == 3) { // Ctrl+C - ASCII 3
            setRawMode(0);
            exit(0);
        } else if (c == 4) { // Ctrl+D - ASCII 4
            setRawMode(0);
            return -1;
        } else if (c == 27) { // ESC - ASCII 27 (arrow keys, function keys)
            // Read and discard the entire escape sequence
            char next_char;
            // Read the second character of escape sequence
            if (read(STDIN_FILENO, &next_char, 1) == 1) {
                if (next_char == '[') {
                    // Continue reading until we get a letter character
                    while (read(STDIN_FILENO, &next_char, 1) == 1) {
                        // Escape sequences end with a letter (A-Z)
                        if (next_char >= 'A' && next_char <= 'Z') {
                            break;
                        }
                        // Also break on numbers to prevent infinite loop
                        if (next_char >= '0' && next_char <= '9') {
                            // Continue reading until we get a letter
                            continue;
                        }
                    }
                }
                // For other escape sequences (like ESC O for F1-F4), just discard
            }
            continue;
        } else if (isAllowedChar(c)) { // Allowed characters
            if (pos < size - 1) {
                buffer[pos++] = c;
                printf("%c", c);
                fflush(stdout);
            }
        }
        // ALL other ASCII characters are ignored (digits, symbols, control chars, etc.)
    }
    
    buffer[pos] = '\0';
    setRawMode(0);
    return pos;
}

int main() {
    char inputBuf[BUFFER_SIZE] = {0};

    init();
    printf("Enter strings (letters and spaces only). Enter '.' to print results.\n");

    while (1) {
        printf("Enter string: ");
        fflush(stdout);
        
        int length = readFilteredLine(inputBuf, BUFFER_SIZE);
        
        if (length == -1) { // Ctrl+D
            printf("\nProgram terminated.\n");
            break;
        }
        
        if (inputBuf[0] == '.' && length == 1) {
            printf("Result:\n");
            printList();
            return 0;
        }
        
        if (length > 0) {
            push(inputBuf);
        } else {
            printf("Empty string not added.\n");
        }
    }

    return 0;
}
