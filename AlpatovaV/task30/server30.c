#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define SOCKET_PATH "./socket30"

int main(){

    unlink(SOCKET_PATH);
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    listen(sock, 1);

    int client = accept(sock, NULL, NULL);

    char c;
    while (read(client, &c, 1) > 0) {
        c = toupper((unsigned char)c);
        write(STDOUT_FILENO, &c, 1);
    }
    write(STDOUT_FILENO, "\n", 1);

    close(client);
    close(sock);
    unlink(SOCKET_PATH);
    return 0;
}
