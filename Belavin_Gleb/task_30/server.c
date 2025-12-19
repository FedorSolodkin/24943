#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "./my_socket"
#define BUF_SIZE 256

int main() {
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char buf[BUF_SIZE];
    int n, i;
    
    unlink(SOCKET_PATH);
    
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }
    
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);
    
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }
    
    if (listen(server_fd, 1) < 0) {
        perror("listen");
        exit(1);
    }
    
    printf("Server waiting...\n");
    
    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("accept");
        exit(1);
    }
    
    printf("Client connected\n");
    
    while ((n = read(client_fd, buf, BUF_SIZE)) > 0) {
        for (i = 0; i < n; i++) {
            buf[i] = toupper(buf[i]);
        }
        write(STDOUT_FILENO, buf, n);
    }
    
    printf("\nClient disconnected\n");
    
    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);
    
    return 0;
}
