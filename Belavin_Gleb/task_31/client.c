#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "./my_socket"

int main(int argc, char *argv[]) {
    int sock_fd;
    struct sockaddr_un addr;
    char buf[64];
    int id, count, i;
    
    if (argc < 3) {
        printf("Usage: %s <client_id> <msg_count>\n", argv[0]);
        exit(1);
    }
    
    id = atoi(argv[1]);
    count = atoi(argv[2]);
    
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
    
    for (i = 0; i < count; i++) {
        sprintf(buf, "client%d_msg%d\n", id, i);
        write(sock_fd, buf, strlen(buf));
        usleep(100000); /* 0.1 sec */
    }
    
    close(sock_fd);
    return 0;
}
