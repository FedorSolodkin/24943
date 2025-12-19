#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/threaded_upper_socket"
#define BUF_SIZE 1024

void *client_handler(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);

    char buf[BUF_SIZE];
    ssize_t n;

    while ((n = read(client_fd, buf, BUF_SIZE)) > 0) {
        for (ssize_t i = 0; i < n; i++)
            buf[i] = toupper((unsigned char)buf[i]);
        write(STDOUT_FILENO, buf, n);
    }

    close(client_fd);
    return NULL;
}

int main() {
    int server_fd;
    struct sockaddr_un addr;

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    unlink(SOCKET_PATH);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("сервер запущен\n");

    while (1) {
        int *client_fd = malloc(sizeof(int));
        if (!client_fd) {
            perror("malloc");
            continue;
        }

        *client_fd = accept(server_fd, NULL, NULL);
        if (*client_fd == -1) {
            perror("accept");
            free(client_fd);
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, client_fd);
        pthread_detach(tid);
    }

    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}
