#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define SOCKET_PATH "./socket30"

int main(){
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    connect(sock, (struct sockaddr*)&addr, sizeof(addr));

    const char* msg = "Hi WoRlD!";
    write(sock, msg, strlen(msg));

    close(sock);
    return 0;
}
