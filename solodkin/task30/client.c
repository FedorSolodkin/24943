#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define SOCKET_NAME "./наш_сокет"

int main() {
    char msg[256];
    int sock, n;
    
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket error");
        return 1;
    }
    
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_NAME);
    
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect error");
        close(sock);
        return 1;
    }
    
    printf("Connected to server. Enter text (Ctrl+D to exit):\n");
    
    while ((n = read(STDIN_FILENO, msg, sizeof(msg))) > 0) {
        if (write(sock, msg, n) != n) {
            perror("write error");
            break;
        }
    }
    
    printf("Client exiting.\n");
    close(sock);
    
    return 0;
}