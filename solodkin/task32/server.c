#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <aio.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>

#define BUFFER_CAPACITY 100
#define MAX_PENDING_CONNECTIONS 5

const char *SOCKET_FILE_PATH = "./socket32";
struct timeval program_start_time;

struct aiocb *setup_async_operation(int client_fd) {
    struct aiocb *async_control = calloc(1, sizeof(struct aiocb));
    if (!async_control) {
        perror("Memory allocation failed for aiocb");
        return NULL;
    }

    async_control->aio_fildes = client_fd;
    async_control->aio_buf = malloc(BUFFER_CAPACITY);
    if (!async_control->aio_buf) {
        free(async_control);
        perror("Memory allocation failed for buffer");
        return NULL;
    }
    
    async_control->aio_nbytes = BUFFER_CAPACITY;
    async_control->aio_offset = 0;

    async_control->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    async_control->aio_sigevent.sigev_signo = SIGIO;
    async_control->aio_sigevent.sigev_value.sival_ptr = async_control;

    return async_control;
}

void process_async_signal(int signum, siginfo_t *info, void *context) {
    if (signum != SIGIO || info->si_signo != SIGIO) {
        return;
    }

    struct aiocb *current_request = info->si_value.sival_ptr;
    int error_status = aio_error(current_request);
    
    if (error_status == 0) {
        ssize_t bytes_read = aio_return(current_request);
        char *data_buffer = (char *)current_request->aio_buf;
        int client_fd = current_request->aio_fildes;

        if (bytes_read == 0) {
            struct timeval current_time;
            gettimeofday(&current_time, NULL);
            double time_elapsed = 
                (current_time.tv_sec - program_start_time.tv_sec) + 
                (current_time.tv_usec - program_start_time.tv_usec) / 1000000.0;
            
            printf("[%.6f] Client %d closed connection\n", 
                   time_elapsed, client_fd);
            
            close(client_fd);
            free(data_buffer);
            free(current_request);
            return;
        }

        if (bytes_read > 0) {
            struct timeval current_time;
            gettimeofday(&current_time, NULL);
            double time_elapsed = 
                (current_time.tv_sec - program_start_time.tv_sec) + 
                (current_time.tv_usec - program_start_time.tv_usec) / 1000000.0;
            
            printf("[%.6f] Client %d: ", time_elapsed, client_fd);
            
            for (ssize_t i = 0; i < bytes_read; i++) {
                data_buffer[i] = toupper((unsigned char)data_buffer[i]);
                printf("%c", data_buffer[i]);
            }
            printf("\n");
            fflush(stdout);

            if (aio_read(current_request) == -1) {
                perror("Failed to restart async read");
                close(client_fd);
                free(data_buffer);
                free(current_request);
            }
        }
    } else if (error_status != EINPROGRESS) {
        perror("Async operation error");
        close(current_request->aio_fildes);
        free(current_request->aio_buf);
        free(current_request);
    }
}

int main(void) {
    int server_socket, client_socket;
    
    gettimeofday(&program_start_time, NULL);

    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    strncpy(server_address.sun_path, SOCKET_FILE_PATH, 
            sizeof(server_address.sun_path) - 1);

    unlink(SOCKET_FILE_PATH);

    if (bind(server_socket, (struct sockaddr *)&server_address, 
             sizeof(server_address)) == -1) {
        perror("Failed to bind socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, MAX_PENDING_CONNECTIONS) == -1) {
        perror("Failed to listen on socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    struct sigaction sigio_action;
    memset(&sigio_action, 0, sizeof(sigio_action));
    sigio_action.sa_sigaction = process_async_signal;
    sigio_action.sa_flags = SA_SIGINFO | SA_RESTART;
    
    if (sigaction(SIGIO, &sigio_action, NULL) == -1) {
        perror("Failed to set signal handler");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Asynchronous server started on %s\n", SOCKET_FILE_PATH);
    printf("Waiting for client connections...\n");

    while (1) {
        client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("Failed to accept client");
            continue;
        }

        struct timeval current_time;
        gettimeofday(&current_time, NULL);
        double time_elapsed = 
            (current_time.tv_sec - program_start_time.tv_sec) + 
            (current_time.tv_usec - program_start_time.tv_usec) / 1000000.0;
        
        printf("[%.6f] New connection: fd=%d\n", time_elapsed, client_socket);

        struct aiocb *async_request = setup_async_operation(client_socket);
        if (!async_request) {
            close(client_socket);
            continue;
        }

        if (aio_read(async_request) == -1) {
            perror("Failed to initiate async read");
            free(async_request->aio_buf);
            free(async_request);
            close(client_socket);
        }
    }

    close(server_socket);
    return EXIT_SUCCESS;
}