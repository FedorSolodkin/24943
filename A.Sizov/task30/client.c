#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/uds_socket"
#define BUFFER_SIZE 1024

int main() {
    int sock_fd;
    struct sockaddr_un addr;
    char buf[BUFFER_SIZE];

    if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // Подключение к серверу
    if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        exit(1);
    }

    printf("Enter text to send to server (Ctrl+D to end):\n");

    // Чтение и отправка на сервер
    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        write(sock_fd, buf, strlen(buf));
    }

    close(sock_fd);
    return 0;
}
