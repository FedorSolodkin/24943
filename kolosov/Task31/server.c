#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>

#define SOCK_PATH "socket_path"

int main(void)
{
    int srv, cli, newfd;
    int clients[FD_SETSIZE];
    int client_ids[FD_SETSIZE];
    int msg_count[FD_SETSIZE];
    int next_id = 1;

    int i, n, maxfd;
    struct sockaddr_un addr;
    fd_set rfds;
    struct timeval tv;

    char buf[1024];
    char outbuf[200000];
    int outlen = 0;

    time_t start;
    struct timeval start_tv;
    struct timeval first, last;
    int have_first = 0;

    for (i = 0; i < FD_SETSIZE; i++) {
        clients[i] = -1;
        client_ids[i] = 0;
        msg_count[i] = 0;
    }

    srv = socket(AF_UNIX, SOCK_STREAM, 0);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCK_PATH);
    unlink(SOCK_PATH);

    bind(srv, (struct sockaddr *)&addr, sizeof(addr));
    listen(srv, 5);

    start = time(NULL);
    gettimeofday(&start_tv, NULL);

    while (1) {
        if (time(NULL) - start >= 15)
            break;

        FD_ZERO(&rfds);
        FD_SET(srv, &rfds);
        maxfd = srv;

        for (i = 0; i < FD_SETSIZE; i++) {
            cli = clients[i];
            if (cli >= 0) {
                FD_SET(cli, &rfds);
                if (cli > maxfd)
                    maxfd = cli;
            }
        }

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        n = select(maxfd + 1, &rfds, NULL, NULL, &tv);
        if (n <= 0)
            continue;

        if (FD_ISSET(srv, &rfds)) {
            newfd = accept(srv, NULL, NULL);
            if (newfd >= 0) {
                for (i = 0; i < FD_SETSIZE; i++) {
                    if (clients[i] < 0) {
                        clients[i] = newfd;
                        client_ids[i] = next_id++;
                        msg_count[i] = 0;
                        break;
                    }
                }
            }
        }

        for (i = 0; i < FD_SETSIZE; i++) {
            cli = clients[i];
            if (cli >= 0 && FD_ISSET(cli, &rfds)) {
                struct timeval now;
                long sec, usec;
                double t;
                int len, j;
                int cid;

                n = read(cli, buf, sizeof(buf));
                if (n <= 0) {
                    close(cli);
                    clients[i] = -1;
                    client_ids[i] = 0;
                    msg_count[i] = 0;
                    continue;
                }

                gettimeofday(&now, NULL);

                if (!have_first) {
                    first = now;
                    have_first = 1;
                }
                last = now;

                sec = now.tv_sec - start_tv.tv_sec;
                usec = now.tv_usec - start_tv.tv_usec;
                if (usec < 0) {
                    sec--;
                    usec += 1000000;
                }

                cid = client_ids[i];
                msg_count[i]++;

                if (outlen < 200000) {
                    len = snprintf(outbuf + outlen,
                                   200000 - outlen,
                                   "[%ld.%06ld] %d message from (%d client): ",
                                   sec, usec, msg_count[i], cid);
                    if (len > 0)
                        outlen += len;
                    if (outlen > 200000)
                        outlen = 200000;
                }

                for (j = 0; j < n && outlen < 200000; j++) {
                    char c = buf[j];
                    c = (char)toupper((unsigned char)c);
                    outbuf[outlen++] = c;
                }

                if (outlen < 200000) {
                    outbuf[outlen++] = '\n';
                }
            }
        }
    }

    if (outlen > 0)
        write(1, outbuf, outlen);

    if (have_first) {
        long sec = last.tv_sec - first.tv_sec;
        long usec = last.tv_usec - first.tv_usec;
        if (usec < 0) {
            sec--;
            usec += 1000000;
        }
        printf("%ld.%06ld s\n", sec, usec);
    }

    close(srv);
    unlink(SOCK_PATH);
    return 0;
}
