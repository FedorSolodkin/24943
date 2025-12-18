#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>

#define SOC_NAME "socket"

int soc;
sig_atomic_t stop = 0;

void assert_noerr(int retcode, const char *msg)
{
    if (retcode < 0)
    {
        fprintf(stderr, "%s\nError: %s\n", msg, strerror(errno));
        exit(1);
    }
}

void exit_handler()
{
    close(soc);
    unlink(SOC_NAME);
}

// void signal_handler(int sig)
// {
//     if (sig == SIGPIPE)
//     {
//         close(soc);
//         unlink(soc);
//         stop = 1;
//     }
// }

int main(int argc, char const *argv[])
{
    // struct sigaction sa;
    // sa.sa_handler = signal_handler;
    // sigemptyset(&sa.sa_mask);
    // sigaction(SIGPIPE, &sa, NULL);

    int ret;
    soc = socket(AF_UNIX, SOCK_STREAM, 0);
    assert_noerr(soc, "Couldn't create a socket.");

    struct sockaddr_un addr;
    socklen_t addr_s = sizeof(addr);
    memset(&addr, 0, addr_s);
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOC_NAME, sizeof(addr.sun_path)-1);

    ret = bind(soc, (struct sockaddr*)&addr, addr_s);
    assert_noerr(ret, "Couldn't bind socket to a local file. Probably it already exists.");
    ret = listen(soc, 5);
    assert_noerr(ret, "`listen` error.\n");
    int con = accept(soc, (struct sockaddr*)&addr, &addr_s);

    char buf;
    while (1)
    {
        ret = read(con, &buf, 1);
        if (ret == 0)
        {
            close(con);
            close(soc);
            unlink(SOC_NAME);
            break;
        }
        buf = toupper(buf);
        write(STDOUT_FILENO, &buf, 1);
    }
    
    return 0;
}
