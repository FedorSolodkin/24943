#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/select.h>

#define SOCKET_NAME "./наш_сокет"
#define MAX_CLIENTS 10  
#define BUFFER_SIZE 256

int main() {
    int server_fd, client_fd;
    int client_sockets[MAX_CLIENTS] = {0};  
    int max_fd; 
    fd_set read_fds; 
    char buffer[BUFFER_SIZE];
    
  
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket error");
        return 1;
    }
    
  
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_NAME);
    

    unlink(SOCKET_NAME);
    
  
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind error");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0) {  
        perror("listen error");
        close(server_fd);
        return 1;
    }
    
    printf("Server started. Waiting for clients...\n");
    printf("Socket file: %s\n", SOCKET_NAME);
    printf("Maximum clients: %d\n", MAX_CLIENTS);
    printf("Press Ctrl+C to stop server\n\n");
    

    while (1) {
   
        FD_ZERO(&read_fds);
        
    
        FD_SET(server_fd, &read_fds);
        max_fd = server_fd;
        
     
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] > 0) {
                FD_SET(client_sockets[i], &read_fds);
                if (client_sockets[i] > max_fd) {
                    max_fd = client_sockets[i];
                }
            }
        }
        
 
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        
        if (activity < 0) {
            perror("select error");
            continue;
        }
        
  
        if (FD_ISSET(server_fd, &read_fds)) {
          
            client_fd = accept(server_fd, NULL, NULL);
            if (client_fd < 0) {
                perror("accept error");
                continue;
            }
            
       
            int client_added = 0;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = client_fd;
                    client_added = 1;
                    printf("New client connected! Client ID: %d (socket: %d)\n", i, client_fd);
                    printf("Total clients: ");
                    int count = 0;
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (client_sockets[j] > 0) count++;
                    }
                    printf("%d\n", count);
                    break;
                }
            }
            
            
            if (!client_added) {
                printf("Too many clients! Rejecting connection.\n");
                close(client_fd);
            }
        }
        
     
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];
            
            if (sd > 0 && FD_ISSET(sd, &read_fds)) {
         
                int n = read(sd, buffer, BUFFER_SIZE);
                
                if (n == 0) {
               
                    printf("Client %d disconnected.\n", i);
                    close(sd);
                    client_sockets[i] = 0;
                } else if (n > 0) {
        
                    for (int j = 0; j < n; j++) {
                        buffer[j] = toupper(buffer[j]);
                    }
                    
                  
                    printf("[Client %d]: ", i);
                    fwrite(buffer, 1, n, stdout);
                    fflush(stdout);
                } else {
                    
                    perror("read error");
                    close(sd);
                    client_sockets[i] = 0;
                }
            }
        }
    }
    

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] > 0) {
            close(client_sockets[i]);
        }
    }
    
    close(server_fd);
    unlink(SOCKET_NAME);
    
    return 0;
}