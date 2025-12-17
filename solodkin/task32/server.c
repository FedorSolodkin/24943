#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <aio.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#define SOCK_FILE "my_socket"
#define MAX_CONNS 20
#define CHUNK_SIZE 1

typedef struct {
    int fd;
    struct aiocb aio_ctrl;
    char single_byte;
    int is_active;
    int client_num;
} active_client;

static active_client client_pool[MAX_CONNS];

void emit_timestamp()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    char tbuf[64];
    strftime(tbuf, sizeof(tbuf), "%H:%M:%S", localtime(&ts.tv_sec));
    printf("[%s.%03ld] ", tbuf, ts.tv_nsec / 1000000);
}

static void graceful_shutdown(int signal_code)
{
    for (int i = 0; i < MAX_CONNS; i++) {
        if (client_pool[i].fd != -1) {
            aio_cancel(client_pool[i].fd, NULL);
            close(client_pool[i].fd);
        }
    }
    unlink(SOCK_FILE);
    exit(0);
}

int main(void)
{
    int listener, new_conn;
    struct sockaddr_un srv_addr;
    int free_slot;

    signal(SIGINT, graceful_shutdown);
    signal(SIGTERM, graceful_shutdown);

    listener = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listener == -1) {
        perror("socket error");
        exit(1);
    }

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sun_family = AF_UNIX;
    strncpy(srv_addr.sun_path, SOCK_FILE, sizeof(srv_addr.sun_path) - 1);
    unlink(SOCK_FILE);

    if (bind(listener, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) == -1) {
        perror("bind failed");
        exit(1);
    }

    if (listen(listener, 10) == -1) {
        perror("listen error");
        exit(1);
    }

    fcntl(listener, F_SETFL, O_NONBLOCK);

    printf("Task 32 server — POSIX AIO + CHAOTIC BYTE MIXING (with timestamps) running...\n");

    for (int i = 0; i < MAX_CONNS; i++) {
        client_pool[i].fd = -1;
        client_pool[i].client_num = i + 1;
    }

    while (1)
    {
        while ((new_conn = accept(listener, NULL, NULL)) != -1)
        {
            for (free_slot = 0; free_slot < MAX_CONNS; free_slot++) {
                if (client_pool[free_slot].fd == -1) break;
            }

            if (free_slot == MAX_CONNS) {
                close(new_conn);
                continue;
            }

            client_pool[free_slot].fd = new_conn;
            client_pool[free_slot].is_active = 1;

            memset(&client_pool[free_slot].aio_ctrl, 0, sizeof(struct aiocb));
            client_pool[free_slot].aio_ctrl.aio_fildes = new_conn;
            client_pool[free_slot].aio_ctrl.aio_buf = &client_pool[free_slot].single_byte;
            client_pool[free_slot].aio_ctrl.aio_nbytes = CHUNK_SIZE;

            if (aio_read(&client_pool[free_slot].aio_ctrl) == -1) {
                perror("aio_read setup failed");
                close(new_conn);
                client_pool[free_slot].fd = -1;
                client_pool[free_slot].is_active = 0;
            }
        }

        for (int i = 0; i < MAX_CONNS; i++)
        {
            if (!client_pool[i].is_active) continue;

            int aio_status = aio_error(&client_pool[i].aio_ctrl);
            if (aio_status == EINPROGRESS) continue;

            ssize_t result = aio_return(&client_pool[i].aio_ctrl);

            if (result == 1)
            {
                char upper_char = toupper((unsigned char)client_pool[i].single_byte);
                printf("%c", upper_char);

                if (aio_read(&client_pool[i].aio_ctrl) == -1) {
                    if (errno != EAGAIN && errno != EINTR) {
                        close(client_pool[i].fd);
                        client_pool[i].fd = -1;
                        client_pool[i].is_active = 0;
                    }
                }
            }
            else
            {
                close(client_pool[i].fd);
                client_pool[i].fd = -1;
                client_pool[i].is_active = 0;
            }
        }

        usleep(1000); // 1ms
    }

    return 0;
}