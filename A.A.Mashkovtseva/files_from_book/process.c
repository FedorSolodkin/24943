#define NUMGROUPS 16
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <grp.h>
#include <sys/resource.h>
#include <sys/time.h>

int main()
{
    gid_t groups[NUMGROUPS];
    struct group *grp;


    struct rlimit rlp;

    printf("terminal is: %s\n",ttyname(0));
    printf("pid = %ld, ppid = %ld\n", getpid(), getppid());
    printf("uid = %ld, euid = %ld\n\n", getuid(), geteuid());

    gid_t gid = getgid();
    grp = getgrgid(gid);
    printf("primary group is: %s\n", grp->gr_name);
    
    int num = getgroups(NUMGROUPS, groups);

    getrlimit(RLIMIT_NOFILE, &rlp);
    printf("open files soft limit: %4ld\n", rlp.rlim_cur);
    printf("open files hard limit: %4ld\n", rlp.rlim_max);
    rlp.rlim_cur = 5;
    setrlimit(RLIMIT_NOFILE, &rlp);
    getrlimit(RLIMIT_NOFILE, &rlp);
    printf("new soft limit: %4ld\n", rlp.rlim_cur);
    exit(0);
}