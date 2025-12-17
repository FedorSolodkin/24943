#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define UNIX_SOCK_NAME "my_socket"
#define MAX_BUFFER 1024

int main(void)
{
    int sock_desc;
    struct sockaddr_un socket_addr;
    char io_buffer[MAX_BUFFER];
    ssize_t bytes_transferred;

    sock_desc = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_desc == -1) {
        perror("socket creation failed");
        exit(1);
    }

    memset(&socket_addr, 0, sizeof(socket_addr));
    socket_addr.sun_family = AF_UNIX;
    strncpy(socket_addr.sun_path, UNIX_SOCK_NAME, sizeof(socket_addr.sun_path) - 1);

    if (connect(sock_desc, (struct sockaddr *)&socket_addr, sizeof(socket_addr)) == -1) {
        perror("connection failed");
        exit(1);
    }

    while ((bytes_transferred = read(STDIN_FILENO, io_buffer, sizeof(io_buffer))) > 0) {
        if (write(sock_desc, io_buffer, bytes_transferred) != bytes_transferred) {
            perror("write error");
            break;
        }
    }

    close(sock_desc);
    return 0;
}