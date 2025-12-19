#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    FILE *fp;
    uid_t uid;

    if(argc < 2){
        fprintf(stderr,"Usage: %s file_name\n", argv[0]);
        exit(1);
    }
    printf("initially uid=%u and euid=%u\n", getuid(), geteuid());
    if ((fp = fopen(argv[1], "r")) == NULL) {
        perror(argv[0]);
        exit(2);
    }
    else {
        printf("first open successful\n");
        fclose(fp);
    }

    setuid( uid = getuid() );

    printf("after setuid(%u):\n   uid=%u and euid=%u\n",
        uid, getuid(), geteuid() );
    if ((fp = fopen(argv[1], "r")) == NULL) {
        perror(argv[0]);
        exit(3);
    }
    else {
        printf("second open successful\n");
        fclose(fp);
    }
    return 0;
}
