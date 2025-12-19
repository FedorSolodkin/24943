#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <errno.h>

#define SOCKET_PATH "/tmp/upper_socket"
#define BUF_SIZE 1024

int main() {
    int server_fd, max_fd, activity, new_socket;
    int client_sockets[FD_SETSIZE];
    struct sockaddr_un addr;
    fd_set readfds;
    char buf[BUF_SIZE];

    // Создание сокета
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    unlink(SOCKET_PATH);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path)-1);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Инициализация массива клиентов
    for (int i = 0; i < FD_SETSIZE; i++) client_sockets[i] = -1;

    printf("Сервер запущен\n");

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_fd = server_fd;

        for (int i = 0; i < FD_SETSIZE; i++) {
            if (client_sockets[i] > 0)
                FD_SET(client_sockets[i], &readfds);
            if (client_sockets[i] > max_fd)
                max_fd = client_sockets[i];
        }

        activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        // Новое соединение
        if (FD_ISSET(server_fd, &readfds)) {
            if ((new_socket = accept(server_fd, NULL, NULL)) == -1) {
                perror("accept");
            } else {
                for (int i = 0; i < FD_SETSIZE; i++) {
                    if (client_sockets[i] == -1) {
                        client_sockets[i] = new_socket;
                        break;
                    }
                }
            }
        }


        // Обработка данных от клиентов
        for (int i = 0; i < FD_SETSIZE; i++) {
            int sd = client_sockets[i];
            if (sd > 0 && FD_ISSET(sd, &readfds)) {
                int n = read(sd, buf, BUF_SIZE);
                if (n <= 0) {
                    close(sd);
                    client_sockets[i] = -1;
                } else {
                    for (int j = 0; j < n; j++)
                        buf[j] = toupper((unsigned char)buf[j]);
                    write(STDOUT_FILENO, buf, n);
                }
            }
        }
    }

    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}
