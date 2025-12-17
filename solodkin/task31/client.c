#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/socket_solodkin_v1"
#define UNIX_SOCK_ADDR "/tmp/uppercase_socket"
#define MAX_BUF 1024

int main(void)
{
    int sock_fd;
    struct sockaddr_un srv_addr;
    char data_buf[MAX_BUF];
    ssize_t num_bytes;

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sun_family = AF_UNIX;
    strncpy(srv_addr.sun_path, UNIX_SOCK_ADDR, sizeof(srv_addr.sun_path) - 1);

    if (connect(sock_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) == -1) {
        perror("connect error");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    printf("Соединение с сервером установлено. Вводите текст (Ctrl+D — выход):\n");

    while ((num_bytes = read(STDIN_FILENO, data_buf, MAX_BUF)) > 0) {
        if (write(sock_fd, data_buf, num_bytes) == -1) {
            perror("write error");
            break;
        }
    }

    if (num_bytes == -1) {
        perror("read error");
    }

    printf("Соединение завершено\n");
    close(sock_fd);
    return 0;
}