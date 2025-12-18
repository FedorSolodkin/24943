#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stropts.h>

#define SOCKET_PATH "./my_socket"
#define BUF_SIZE 256
#define MAX_CLIENTS 10

int server_fd;
int clients[MAX_CLIENTS];
int num_clients = 0;
int had_clients = 0;
volatile int should_exit = 0;

void process_io(int sig) {
    char buf[BUF_SIZE];
    int i, j, n;
    int client_fd;
    
    /* Try accept new connection */
    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd >= 0) {
        if (num_clients < MAX_CLIENTS) {
            fcntl(client_fd, F_SETFL, O_NONBLOCK);
            fcntl(client_fd, F_SETOWN, getpid());
            ioctl(client_fd, I_SETSIG, S_INPUT);
            clients[num_clients++] = client_fd;
            had_clients = 1;
            printf("Client %d connected\n", client_fd);
        } else {
            close(client_fd);
        }
    }
    
    /* Read from all clients */
    for (i = 0; i < num_clients; i++) {
        while ((n = read(clients[i], buf, BUF_SIZE)) > 0) {
            for (j = 0; j < n; j++)
                buf[j] = toupper(buf[j]);
            write(STDOUT_FILENO, buf, n);
        }
        
        if (n == 0) {
            printf("Client %d disconnected\n", clients[i]);
            close(clients[i]);
            for (j = i; j < num_clients - 1; j++)
                clients[j] = clients[j + 1];
            num_clients--;
            i--;
        }
    }
    
    /* Exit when all clients disconnected */
    if (had_clients && num_clients == 0) {
        should_exit = 1;
    }
}

int main() {
    struct sockaddr_un addr;
    struct sigaction sa;
    
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
    
    sa.sa_handler = process_io;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGPOLL, &sa, NULL);
    
    fcntl(server_fd, F_SETFL, O_NONBLOCK);
    fcntl(server_fd, F_SETOWN, getpid());
    ioctl(server_fd, I_SETSIG, S_INPUT);
    
    printf("Server waiting (async)...\n");
    
    while (!should_exit) {
        pause();
    }
    
    printf("All clients disconnected. Exiting.\n");
    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}
