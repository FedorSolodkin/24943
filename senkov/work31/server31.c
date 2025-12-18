#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>

#define SOCKET_PATH "/tmp/uds_socket_31"
#define BUFFER_SIZE 256

void to_upper_case(char *str) {
    if (str == NULL) return;
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        str[i] = (char)toupper((unsigned char)str[i]);
    }
}

int main() {
    int listen_fd, max_fd;
    struct sockaddr_un server_addr;
    fd_set read_fds, temp_fds;
    char buffer[BUFFER_SIZE];
    int fd;

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

    printf("Сервер 31: Слушаю на %s\n", SOCKET_PATH);

    FD_ZERO(&read_fds);
    FD_SET(listen_fd, &read_fds);
    max_fd = listen_fd;

    while (1) {
        temp_fds = read_fds;
        int activity = select(max_fd + 1, &temp_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(listen_fd, &temp_fds)) {
            int client_fd = accept(listen_fd, NULL, NULL);
            if (client_fd == -1) {
                perror("accept");
                continue;
            }
            printf("Клиент подключился (fd=%d)\n", client_fd);
            FD_SET(client_fd, &read_fds);
            if (client_fd > max_fd) max_fd = client_fd;
        }

        for (fd = 0; fd <= max_fd; fd++) {
            if (fd != listen_fd && FD_ISSET(fd, &temp_fds)) {
                ssize_t bytes_read = recv(fd, buffer, BUFFER_SIZE - 1, 0);
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0';
                    printf("Получено: \"%s\"\n", buffer);
                    to_upper_case(buffer);
                    printf("Верхний регистр: \"%s\"\n", buffer);
                } else {
                    printf("Клиент (fd=%d) отключился\n", fd);
                    close(fd);
                    FD_CLR(fd, &read_fds);
                }
            }
        }
    }

    close(listen_fd);
    unlink(SOCKET_PATH);
    return 0;
}
