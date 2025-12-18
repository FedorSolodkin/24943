#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define SOCK_PATH "./socket.sock"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <char> <count>\n", argv[0]);
        return 1;
    }

    char c = argv[1][0];
    int count = atoi(argv[2]);

    for (int i = 0; i < count; i++) {
        int sock = socket(AF_UNIX, SOCK_STREAM, 0);
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);
        socklen_t len = sizeof(sa_family_t) + strlen(addr.sun_path) + 1;
        connect(sock, (struct sockaddr*)&addr, len);

        write(sock, &c, 1);
        write(sock, "\n", 1);
        close(sock);

        usleep(50000);
    }
    return 0;
}
