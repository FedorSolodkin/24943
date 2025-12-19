#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/uds_socket_30"
#define BUFFER_SIZE 256

// Функция для перевода строки в верхний регистр
void to_upper_case(char *str) {
    int i;
    if (str == NULL) return;
    for (i = 0; str[i] != '\0'; i++) { 
        str[i] = (char)toupper((unsigned char)str[i]);
    }
}

int main() {
    int listen_fd, client_fd;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // 1. Удаление старого файла сокета для предотвращения ошибки "Address already in use"
    unlink(SOCKET_PATH); 

    // 2. Создание сокета
    if ((listen_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 3. Привязка (Bind)
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        goto cleanup_listen;
    }
    printf("Сервер: Слушаю на %s\n", SOCKET_PATH);

    // 4. Прослушивание и принятие соединения
    if (listen(listen_fd, 5) == -1) {
        perror("Listen failed");
        goto cleanup_listen;
    }

    if ((client_fd = accept(listen_fd, NULL, NULL)) == -1) {
        perror("Accept failed");
        goto cleanup_listen;
    }
    printf("Сервер: Клиент подключился.\n");

    // 5. Получение, обработка и вывод данных
    bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("Получено: \"%s\"\n", buffer);
        to_upper_case(buffer);
        printf("Преобразовано и выведено: \"%s\"\n", buffer);
    } else if (bytes_read == 0) {
        printf("Соединение закрыто клиентом.\n");
    } else {
        perror("Receive failed");
    }

    // 6. Завершение и очистка
    close(client_fd);

cleanup_listen:
    close(listen_fd);
    // Удаление файла сокета - гарантия чистого завершения
    unlink(SOCKET_PATH);
    printf("Сервер: Завершение и очистка.\n");

    return (bytes_read > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
