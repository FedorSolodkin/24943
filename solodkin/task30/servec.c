#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SOCKET_NAME "./наш_сокет"

int main() {
    char buf[256];
    int server_fd, client_fd, n;
    
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket error");
        return 1;
    }
    
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_NAME);
    
    unlink(SOCKET_NAME);
    
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind error");
        close(server_fd);
        return 1;
    }
    
    if (listen(server_fd, 1) < 0) {
        perror("listen error");
        close(server_fd);
        return 1;
    }
    
    printf("Server started. Waiting for client...\n");
    
    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("accept error");
        close(server_fd);
        return 1;
    }
    
    printf("Client connected. Receiving text...\n");
    
    while ((n = read(client_fd, buf, sizeof(buf))) > 0) {
        for (int i = 0; i < n; i++) {
            buf[i] = toupper(buf[i]);
        }
        fwrite(buf, 1, n, stdout);
        fflush(stdout);
    }
    
    printf("\nConnection closed by client.\n");
    
    close(client_fd);
    close(server_fd);
    unlink(SOCKET_NAME);
    
    return 0;
}