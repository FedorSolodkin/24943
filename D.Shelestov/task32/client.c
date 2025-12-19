#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>

char *socket_path = "./socket32";

int main(int argc, char *argv[]) {
    int fd;
    struct sockaddr_un addr;

    if (argc < 3) {
        printf("Usage: %s <character> <count>\n", argv[0]);
        exit(1);
    }

    char ch = argv[1][0];
    int count = atoi(argv[2]);

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        exit(-1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect error");
        exit(-1);
    }

    printf("Client connected, sending %d '%c' characters...\n", count, ch);
    for (int i = 0; i < count; i++) {
        write(fd, &ch, 1);
        usleep(100000); // 100ms задержка
    }

    shutdown(fd, SHUT_WR);

    sleep(1);
    close(fd);

    return 0;
}
