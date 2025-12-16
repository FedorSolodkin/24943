#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>

const char *SOCKET_FILE_PATH = "./socket32";

int main(int argc, char *argv[]) {
    int connection_fd;
    struct sockaddr_un server_addr;

    if (argc < 3) {
        printf("Usage: %s <character> <repeat_count>\n", argv[0]);
        exit(1);
    }

    char character_to_send = argv[1][0];
    int transmission_count = atoi(argv[2]);

    connection_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (connection_fd == -1) {
        perror("socket creation failed");
        exit(-1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_FILE_PATH, 
            sizeof(server_addr.sun_path) - 1);

    if (connect(connection_fd, (struct sockaddr*)&server_addr, 
                sizeof(server_addr)) == -1) {
        perror("server connection failed");
        exit(-1);
    }

    printf("Connected to server. Sending %d '%c' characters...\n", 
           transmission_count, character_to_send);
    
    for (int iteration = 0; iteration < transmission_count; iteration++) {
        write(connection_fd, &character_to_send, 1);
        usleep(100000);
    }

    shutdown(connection_fd, SHUT_WR);
    sleep(1);
    close(connection_fd);

    return 0;
}