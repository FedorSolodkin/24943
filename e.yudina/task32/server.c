#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>

#define SOCK_PATH "./socket.sock"

static struct timespec start_time;

void print_time(const char* msg) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double elapsed = (now.tv_sec - start_time.tv_sec) + (now.tv_nsec - start_time.tv_nsec) / 1e9;
    fprintf(stderr, "[%6.3f] %s\n", elapsed, msg);
}

void sigint_handler(int sig) {
    print_time("Server stopped");
    unlink(SOCK_PATH);
    exit(0);
}

void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    unlink(SOCK_PATH);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    print_time("Server started");

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        return 1;
    }
    listen(sock, 5);

    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);
    signal(SIGCHLD, sigchld_handler);

    while (1) {
        int client = accept(sock, NULL, NULL);
        print_time("Client message");

        if (fork() == 0) {
            close(sock);
            char c;
            while (read(client, &c, 1) > 0) {
                putchar(toupper((unsigned char)c));
                fflush(stdout);
            }
            close(client);
            exit(0);
        }
        close(client);
    }
}
