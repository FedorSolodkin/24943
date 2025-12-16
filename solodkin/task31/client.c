#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>

const char *SOCKET_FILE_PATH = "./socket31";

int main(int argc, char *argv[]) {
    int socket_fd;
    struct sockaddr_un server_address;

    if (argc < 3) {
        printf("Usage: %s <character> <repeat_count>\n", argv[0]);
        exit(1);
    }

    char transmit_char = argv[1][0];
    int repeat_count = atoi(argv[2]);

    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("socket creation failed");
        exit(-1);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    strncpy(server_address.sun_path, SOCKET_FILE_PATH, 
            sizeof(server_address.sun_path) - 1);

    if (connect(socket_fd, (struct sockaddr*)&server_address, 
                sizeof(server_address)) == -1) {
        perror("server connection failed");
        exit(-1);
    }

    printf("Connected to server. Transmitting %d '%c' characters...\n", 
           repeat_count, transmit_char);
    
    for (int iteration = 0; iteration < repeat_count; iteration++) {
        write(socket_fd, &transmit_char, 1);
        usleep(100000);
    }

    shutdown(socket_fd, SHUT_WR);
    sleep(1);
    close(socket_fd);

    return 0;
}