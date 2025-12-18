#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/upper_socket"
#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <текст>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sock;
    struct sockaddr_un addr;

    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path)-1);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    char buf[BUF_SIZE];
    buf[0] = '\0';
    for (int i = 1; i < argc; i++) {
        strcat(buf, argv[i]);
        if (i != argc - 1) strcat(buf, " ");
    }

    write(sock, buf, strlen(buf));

    close(sock);
    return 0;
}
