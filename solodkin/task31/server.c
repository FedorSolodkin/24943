#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <poll.h>
#include <sys/time.h>

#define MAX_PENDING_CONNECTIONS 5
#define MAX_POLL_DESCRIPTORS (MAX_PENDING_CONNECTIONS + 1)
#define SOCKET_PATH "/tmp/socket_solodkin_v1" // Путь должен совпадать с клиентом!

int register_client(struct pollfd *poll_set, int client_fd);

int main() {
    char buffer[1024];
    int server_fd, client_fd;
    ssize_t bytes_read;
    
    struct timeval program_start;
    gettimeofday(&program_start, NULL);

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket creation failed");
        exit(-1);
    }

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // ВАЖНО: Удаляем старый сокет перед привязкой
    unlink(SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("bind failed");
        exit(-1);
    }

    if (listen(server_fd, MAX_PENDING_CONNECTIONS) == -1) {
        perror("listen failed");
        exit(-1);
    }

    struct pollfd monitored_fds[MAX_POLL_DESCRIPTORS];
    for (int idx = 0; idx < MAX_POLL_DESCRIPTORS; idx++) {
        monitored_fds[idx].fd = -1;
        monitored_fds[idx].events = POLLIN | POLLPRI;
    }

    monitored_fds[0].fd = server_fd;
    printf("Server initialized on %s. Awaiting client connections...\n", SOCKET_PATH);

    while (1) {
        if (poll(monitored_fds, MAX_POLL_DESCRIPTORS, -1) == -1) {
            perror("polling error");
            break; // Лучше break чем exit, чтобы дойти до очистки (если бы она была ниже)
        }

        // Проверяем ошибки на дескрипторах
        for (int idx = 0; idx < MAX_POLL_DESCRIPTORS; idx++) {
            if (monitored_fds[idx].fd < 0) continue;
            
            short event_flags = monitored_fds[idx].revents;
            if ((event_flags & POLLERR) || (event_flags & POLLHUP) || (event_flags & POLLNVAL)) {
                if (idx == 0) {
                    printf("Critical server error detected\n");
                    exit(-1);
                } else {
                    close(monitored_fds[idx].fd);
                    monitored_fds[idx].fd = -1;
                }
            }
        }

        // Новые подключения (слушающий сокет)
        if ((monitored_fds[0].revents & POLLIN) || (monitored_fds[0].revents & POLLPRI)) {
            client_fd = accept(server_fd, NULL, NULL);
            if (client_fd != -1) {
                if (register_client(monitored_fds, client_fd) == -1) {
                    printf("Too many clients, closing connection %d\n", client_fd);
                    close(client_fd);
                } else {
                    struct timeval now;
                    gettimeofday(&now, NULL);
                    double time_passed = (now.tv_sec - program_start.tv_sec) +
                                       (now.tv_usec - program_start.tv_usec) / 1000000.0;
                    printf("[%.6f] New client connected with descriptor %d\n", 
                           time_passed, client_fd);
                }
            }
        }

        // Данные от клиентов
        for (int idx = 1; idx < MAX_POLL_DESCRIPTORS; idx++) {
            if (monitored_fds[idx].fd < 0) continue;
            
            if ((monitored_fds[idx].revents & POLLIN) || (monitored_fds[idx].revents & POLLPRI)) {
                bytes_read = read(monitored_fds[idx].fd, buffer, sizeof(buffer));
                
                if (bytes_read > 0) {
                    struct timeval now;
                    gettimeofday(&now, NULL);
                    double time_passed = (now.tv_sec - program_start.tv_sec) +
                                       (now.tv_usec - program_start.tv_usec) / 1000000.0;

                    printf("[%.6f] Client %d transmitted: ", time_passed, monitored_fds[idx].fd);
                    for (int j = 0; j < bytes_read; j++) {
                        // Преобразование только если это печатный символ, иначе может быть мусор при переносе строки
                        if(isprint(buffer[j]) || isspace(buffer[j])) { 
                            printf("%c", toupper((unsigned char)buffer[j]));
                        }
                    }
                    if (buffer[bytes_read-1] != '\n') printf("\n"); // Добавляем перенос если его не было
                    fflush(stdout);
                } else {
                    // 0 байт = клиент закрыл соединение, -1 = ошибка
                    close(monitored_fds[idx].fd);
                    monitored_fds[idx].fd = -1;
                }
            }
        }
    }
    
    unlink(SOCKET_PATH);
    return 0;
}

int register_client(struct pollfd *poll_set, int client_fd) {
    for (int idx = 1; idx < MAX_POLL_DESCRIPTORS; idx++) {
        if (poll_set[idx].fd < 0) {
            poll_set[idx].fd = client_fd;
            poll_set[idx].events = POLLIN | POLLPRI;
            return 0; 
        }
    }
    return -1;
}