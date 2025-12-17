#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    int fd = open("file.txt", O_RDWR);
    if (fcntl(fd, F_SETLK, &lock) == -1) {
        close(fd);
        return 1;
    }

    system("vi file.txt");

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    
    close(fd);
    return 0;
}