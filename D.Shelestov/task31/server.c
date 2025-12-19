#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>
#include <poll.h>
#include <time.h>
#include <sys/time.h>

#define BACKLOG (5)
#define POLL_LENGTH (BACKLOG + 1)

int addConnection(struct pollfd *poll_list, int fd);

char *socket_path = "./socket31";

int main() {
    char buf[100];
    int fd, cl, rc;

    // отсчет времени
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        exit(-1);
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    unlink(socket_path);

    if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("bind error");
        exit(-1);
    }

    if (listen(fd, BACKLOG) == -1) {
        perror("listen error");
        exit(-1);
    }

    struct pollfd poll_fds[POLL_LENGTH];
    for (int i = 0; i < POLL_LENGTH; i++) {
        poll_fds[i].fd = -1;
        poll_fds[i].events = POLLIN | POLLPRI;
    }

    poll_fds[0].fd = fd;
    printf("Server started. Waiting for connections...\n");

    while (1) {
        if ((poll(poll_fds, POLL_LENGTH, -1)) == -1) {
            perror("bad poll");
            exit(-1);
        }

        for (int i = 0; i < POLL_LENGTH; i++) {
            if (poll_fds[i].fd < 0) continue;
            short revents = poll_fds[i].revents;

            if ((revents & POLLERR) || (revents & POLLHUP) || (revents & POLLNVAL)) {
                if (i == 0) {
                    printf("Server error");
                    exit(-1);
                } else {
                    struct timeval current_time;
                    gettimeofday(&current_time, NULL);
                    double elapsed = (current_time.tv_sec - start_time.tv_sec) +
                                    (current_time.tv_usec - start_time.tv_usec) / 1000000.0;
                    printf("[%.6f] Client %d disconnected\n", elapsed, poll_fds[i].fd);
                    close(poll_fds[i].fd);
                    poll_fds[i].fd = -1;
                }
            }
        }

        if ((poll_fds[0].revents & POLLIN) || (poll_fds[0].revents & POLLPRI)) {
            if ((cl = accept(fd, NULL, NULL)) == -1) {
                perror("accept error");
                continue;
            }

            if (addConnection(poll_fds, cl) == -1) {
                perror("Failed to add new connection");
            } else {
                struct timeval current_time;
                gettimeofday(&current_time, NULL);
                double elapsed = (current_time.tv_sec - start_time.tv_sec) +
                                (current_time.tv_usec - start_time.tv_usec) / 1000000.0;
                printf("[%.6f] New client connected: fd=%d\n", elapsed, cl);
            }
        }

        for (int i = 1; i < POLL_LENGTH; i++) {
            if (poll_fds[i].fd < 0) continue;
            int cur_desc = poll_fds[i].fd;

            if ((poll_fds[i].revents & POLLIN) || (poll_fds[i].revents & POLLPRI)) {
                if ((rc = read(cur_desc, buf, sizeof(buf))) > 0) {
                    struct timeval current_time;
                    gettimeofday(&current_time, NULL);
                    double elapsed = (current_time.tv_sec - start_time.tv_sec) +
                                    (current_time.tv_usec - start_time.tv_usec) / 1000000.0;

                    // время, дескриптор клиента и данные
                    printf("[%.6f] Client %d: ", elapsed, cur_desc);
                    for (int j = 0; j < rc; j++) {
                        buf[j] = toupper(buf[j]);
                        printf("%c", buf[j]);
                    }
                    printf("\n");
                    fflush(stdout);
                }

                if (rc == -1) {
                    perror("read");
                    exit(-1);
                } else if (rc == 0) {
                    struct timeval current_time;
                    gettimeofday(&current_time, NULL);
                    double elapsed = (current_time.tv_sec - start_time.tv_sec) +
                                    (current_time.tv_usec - start_time.tv_usec) / 1000000.0;
                    printf("\n[%.6f] Client %d disconnected (EOF)\n", elapsed, cur_desc);
                    close(cur_desc);
                    poll_fds[i].fd = -1;
                }
            }
        }
    }

    return 0;
}

int addConnection(struct pollfd *poll_list, int fd) {
    int result = -1;

    for (int i = 1; i < POLL_LENGTH; i++) {
        if (poll_list[i].fd < 0) {
            poll_list[i].fd = fd;
            poll_list[i].events = POLLIN | POLLPRI;
            result = 0;
            break;
        }
    }

    return result;
}
