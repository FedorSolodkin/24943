#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>

#define SOCKET_PATH "/tmp/uds_socket_32"
#define BUFFER_SIZE 256
#define MAX_CLIENTS 10

void to_upper_case(char *str) {
    if (str == NULL) return;
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        str[i] = (char)toupper((unsigned char)str[i]);
    }
}

int main() {
    int listen_fd;
    struct sockaddr_un server_addr;
    struct pollfd fds[MAX_CLIENTS + 1];
    char buffer[BUFFER_SIZE];
    int nfds = 1;
    int i;

    unlink(SOCKET_PATH);

    if ((listen_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(listen_fd, 5) == -1) {
        perror("listen");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    printf("Сервер 32: Слушаю на %s\n", SOCKET_PATH);

    fds[0].fd = listen_fd;
    fds[0].events = POLLIN;

    for (i = 1; i <= MAX_CLIENTS; i++) {
        fds[i].fd = -1;
        fds[i].events = POLLIN;
    }

    while (1) {
        int ret = poll(fds, nfds, -1);
        if (ret < 0) {
            perror("poll");
            break;
        }

        if (fds[0].revents & POLLIN) {
            int client_fd = accept(listen_fd, NULL, NULL);
            if (client_fd == -1) {
                perror("accept");
                continue;
            }

            int found = 0;
            for (i = 1; i <= MAX_CLIENTS; i++) {
                if (fds[i].fd == -1) {
                    fds[i].fd = client_fd;
                    if (i >= nfds) nfds = i + 1;
                    found = 1;
                    break;
                }
            }
            if (!found) {
                close(client_fd);
                printf("Слишком много клиентов\n");
            }
        }

        for (i = 1; i < nfds; i++) {
            if (fds[i].fd == -1) continue;
            if (fds[i].revents & POLLIN) {
                ssize_t bytes_read = recv(fds[i].fd, buffer, BUFFER_SIZE - 1, 0);
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0';
                    printf("Получено: \"%s\"\n", buffer);
                    to_upper_case(buffer);
                    printf("Верхний регистр: \"%s\"\n", buffer);
                } else {
                    close(fds[i].fd);
                    fds[i].fd = -1;
                }
            }
        }
    }

    close(listen_fd);
    unlink(SOCKET_PATH);
    return 0;
}
