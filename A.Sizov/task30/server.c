#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/uds_socket"
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char buf[BUFFER_SIZE];
    ssize_t n;

    // Unix domain сокет
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    unlink(SOCKET_PATH);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // Привязка
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    // Прослушка
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(1);
    }

    printf("Server listening on %s...\n", SOCKET_PATH);

    // соединение
    if ((client_fd = accept(server_fd, NULL, NULL)) < 0) {
        perror("accept");
        exit(1);
    }

    // Чтение данных с клиента
    while ((n = read(client_fd, buf, BUFFER_SIZE)) > 0) {
        for (ssize_t i = 0; i < n; i++) {
            buf[i] = toupper((unsigned char)buf[i]);
        }
        write(STDOUT_FILENO, buf, n);  // вывод на экран
    }

    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);

    printf("\nConnection closed.\n");
    return 0;
}
