#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>

#define SOCKET_PATH "./my_socket"
#define BUF_SIZE 256
#define MAX_CLIENTS 10

int main() {
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char buf[BUF_SIZE];
    struct pollfd fds[MAX_CLIENTS + 1];
    int nfds = 1;
    int had_clients = 0;
    int i, n, j;
    
    unlink(SOCKET_PATH);
    
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }
    
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);
    
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }
    
    listen(server_fd, 5);
    
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;
    
    printf("Server waiting...\n");
    
    while (1) {
        if (poll(fds, nfds, -1) < 0) {
            perror("poll");
            break;
        }
        
        /* New connection */
        if (fds[0].revents & POLLIN) {
            client_fd = accept(server_fd, NULL, NULL);
            if (client_fd >= 0 && nfds < MAX_CLIENTS + 1) {
                fds[nfds].fd = client_fd;
                fds[nfds].events = POLLIN;
                nfds++;
                had_clients = 1;
                printf("Client %d connected\n", client_fd);
            }
        }
        
        /* Check clients */
        for (i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                n = read(fds[i].fd, buf, BUF_SIZE);
                if (n <= 0) {
                    printf("Client %d disconnected\n", fds[i].fd);
                    close(fds[i].fd);
                    for (j = i; j < nfds - 1; j++)
                        fds[j] = fds[j + 1];
                    nfds--;
                    i--;
                } else {
                    for (j = 0; j < n; j++)
                        buf[j] = toupper(buf[j]);
                    write(STDOUT_FILENO, buf, n);
                }
            }
        }
        
        /* Exit when all clients disconnected */
        if (had_clients && nfds == 1) {
            printf("All clients disconnected. Exiting.\n");
            break;
        }
    }
    
    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}
