#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#define SOCK_PATH "socket.sock"

int main() {
    unlink(SOCK_PATH); // Удаляем старый сокет

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr = {.sun_family = AF_UNIX};
    strcpy(addr.sun_path, SOCK_PATH);
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    listen(sock, 1);

    int client = accept(sock, NULL, NULL);
    char c;
    while (read(client, &c, 1) > 0) {
        putchar(toupper((unsigned char)c));
    }
    putchar('\n');

    close(client);
    close(sock);
    unlink(SOCK_PATH);
    return 0;
}
