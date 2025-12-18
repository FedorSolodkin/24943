#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define SOCK_PATH "socket.sock"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s \"text\"\n", argv[0]);
        return 1;
    }

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr = {.sun_family = AF_UNIX};
    strcpy(addr.sun_path, SOCK_PATH);
    connect(sock, (struct sockaddr*)&addr, sizeof(addr));

    write(sock, argv[1], strlen(argv[1]));
    close(sock);
    return 0;
}
