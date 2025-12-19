#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "./my_socket"
#define BUF_SIZE 256

int main() {
    int sock_fd;
    struct sockaddr_un addr;
    char buf[BUF_SIZE];
    
    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        exit(1);
    }
    
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);
    
    if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        exit(1);
    }
    
    printf("Connected. Enter text (Ctrl+D to end):\n");
    
    while (fgets(buf, BUF_SIZE, stdin) != NULL) {
        write(sock_fd, buf, strlen(buf));
    }
    
    close(sock_fd);
    printf("Disconnected\n");
    
    return 0;
}
