#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define SOC_NAME "socket"

int soc;

void assert_noerr(int retcode, const char *msg)
{
    if (retcode < 0)
    {
        fprintf(stderr, "%s", msg);
        exit(1);
    }
}

void exit_handler()
{
    close(soc);
}

int main(int argc, char const *argv[])
{
    int ret;
    soc = socket(AF_UNIX, SOCK_STREAM, 0);
    assert_noerr(soc, "Couldn't create a socket.\n");

    struct sockaddr_un addr;
    socklen_t addr_s = sizeof(addr);
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOC_NAME, sizeof(addr.sun_path)-1);

    ret = connect(soc, (struct sockaddr*)&addr, addr_s);
    assert_noerr(ret, "Couldn't connect to a socket.\n");

    char buf[64];
    while (1)
    {
        scanf("%s", buf);
        if (strlen(buf) == 0)
        {
            close(soc);
            break;
        }
        write(soc, buf, strlen(buf));
    }
    
    return 0;
}
