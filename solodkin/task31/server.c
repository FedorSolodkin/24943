#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>

#define UNIX_SOCK_PATH "/tmp/uppercase_socket"
#define BUF_SIZE 1024
#define MAX_CONN 10

static volatile sig_atomic_t terminate_flag = 0;

void signal_handler(int signum)
{
    terminate_flag = 1;
    printf("\nПринят сигнал %d. Завершаем сервер...\n", signum);
}

int main(void)
{
    int srv_sock, cli_sock;
    struct sockaddr_un srv_addr, cli_addr;
    socklen_t cli_addr_len;
    struct pollfd poll_fds[MAX_CONN + 1];
    int active_fds = 1;
    int poll_timeout_ms = 1000;
    char recv_buf[BUF_SIZE];
    int idx, received;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    srv_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (srv_sock == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sun_family = AF_UNIX;
    strncpy(srv_addr.sun_path, UNIX_SOCK_PATH, sizeof(srv_addr.sun_path) - 1);

    unlink(UNIX_SOCK_PATH);

    if (bind(srv_sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) == -1) {
        perror("bind failed");
        close(srv_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(srv_sock, 5) == -1) {
        perror("listen failed");
        close(srv_sock);
        exit(EXIT_FAILURE);
    }

    printf("Сервер запущен на %s\n", UNIX_SOCK_PATH);
    printf("Завершить: Ctrl+C или SIGTERM\n");

    memset(poll_fds, 0, sizeof(poll_fds));
    poll_fds[0].fd = srv_sock;
    poll_fds[0].events = POLLIN;

    while (!terminate_flag)
    {
        int poll_result = poll(poll_fds, active_fds, poll_timeout_ms);

        if (poll_result == -1) {
            if (errno == EINTR) continue;
            perror("poll error");
            break;
        }

        if (poll_result == 0) continue;

        if (poll_fds[0].revents & POLLIN)
        {
            cli_addr_len = sizeof(cli_addr);
            cli_sock = accept(srv_sock, (struct sockaddr *)&cli_addr, &cli_addr_len);
            if (cli_sock == -1) {
                perror("accept error");
                continue;
            }

            printf("Новое подключение (дескриптор: %d)\n", cli_sock);

            if (active_fds < MAX_CONN + 1) {
                poll_fds[active_fds].fd = cli_sock;
                poll_fds[active_fds].events = POLLIN;
                active_fds++;
            } else {
                printf("Достигнут лимит подключений\n");
                close(cli_sock);
            }
        }

        for (idx = 1; idx < active_fds; idx++)
        {
            if (poll_fds[idx].revents & POLLIN)
            {
                received = read(poll_fds[idx].fd, recv_buf, BUF_SIZE - 1);

                if (received > 0)
                {
                    recv_buf[received] = '\0';
                    for (int k = 0; k < received; k++) {
                        recv_buf[k] = toupper((unsigned char)recv_buf[k]);
                    }
                    printf("[Клиент %d]: %s", poll_fds[idx].fd, recv_buf);
                    fflush(stdout);
                }

                if (received <= 0)
                {
                    printf("Клиент отключился (дескриптор: %d)\n", poll_fds[idx].fd);
                    close(poll_fds[idx].fd);
                    poll_fds[idx].fd = -1;
                }
            }
        }

        for (idx = 1; idx < active_fds; idx++)
        {
            if (poll_fds[idx].fd == -1)
            {
                for (int shift = idx; shift < active_fds - 1; shift++) {
                    poll_fds[shift] = poll_fds[shift + 1];
                }
                active_fds--;
                idx--;
            }
        }
    }

    printf("Остановка сервера...\n");

    for (idx = 0; idx < active_fds; idx++) {
        if (poll_fds[idx].fd != -1) {
            close(poll_fds[idx].fd);
        }
    }

    unlink(UNIX_SOCK_PATH);
    printf("Сервер завершил работу.\n");

    return 0;
}