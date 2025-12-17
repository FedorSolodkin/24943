#include <ctype.h>
#include <unistd.h>

#define PIPE_READ_END 0
#define PIPE_WRITE_END 1

void process_data(int input_fd) {
    unsigned char symbol;
    while (read(input_fd, &symbol, 1) > 0) {
        symbol = toupper(symbol);
        write(1, &symbol, 1);
    }
    write(1, "\n", 1);
}

void send_message(int output_fd) {
    const char *text = "hello WoRld!";
    int i = 0;
    while (text[i] != '\0') {
        write(output_fd, &text[i], 1);
        i++;
    }
}

int setup_communication(void) {
    int channel[2];
    
    if (pipe(channel) != 0) {
        return -1;
    }
    
    int child_pid = fork();
    if (child_pid < 0) {
        return -1;
    }
    
    if (child_pid != 0) {
        close(channel[PIPE_READ_END]);
        send_message(channel[PIPE_WRITE_END]);
        close(channel[PIPE_WRITE_END]);
    } else {
        close(channel[PIPE_WRITE_END]);
        process_data(channel[PIPE_READ_END]);
        close(channel[PIPE_READ_END]);
    }
    
    return child_pid;
}

int main() {
    int result = setup_communication();
    
    if (result < 0) {
        const char error_msg[] = "Operation failed\n";
        write(2, error_msg, sizeof(error_msg) - 1);
        return 1;
    }
    
    return 0;
}