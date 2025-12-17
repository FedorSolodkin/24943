// #include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

char pattern[] = "rwxrwxrwx";

void print_ld(struct stat *s);


int main(int argc, char const *argv[])
{
    struct stat st;
    
    if (argc < 2)
    {
        fprintf(stderr, "Empty args.\n");
    }

    for (int i = 1; i < argc; i++)
    {
        int ret = stat(argv[i], &st);
        if (ret < 0)
        {
            fprintf(stderr, "Error reading file stat: '%s'\n", argv[i]);
            continue;
        }
        print_ld(&st);
    }
    

    // printf("%ld\n", telldir(dir));
    
    // while ((ent = readdir(dir)) != NULL)
    // {
    //     printf("%ld, %s, %ld, %hd, %c\n", ent->d_ino, ent->d_name, ent->d_off, ent->d_reclen, ent->d_type);
    // }
    
    return 0;
}


void print_ld(struct stat *s)
{
    struct passwd *p = getpwuid(s->st_uid);
    struct group *g = getgrgid(s->st_gid);

    struct tm *tim = localtime(&s->st_mtim.tv_sec);
    time_t clk = time(NULL);
    struct tm *now = localtime(&clk);
    int diff_year = tim->tm_year != now->tm_year ? 1 : 0;

    char time_f[40];

    printf("%c", S_ISDIR(s->st_mode) ? 'd' : '-');

    for (int i = 0; i < 9; i++)
    {
        printf("%c", (s->st_mode >> (8-i) & 1) ? pattern[i] : '-');
    }

    printf(" %5ld %16s %16s %16ld", s->st_nlink, p->pw_name, g->gr_name, s->st_size);
    strftime(time_f, 39, (diff_year ? "%Y %b %d %H:%M" : "%b %d %H:%M"), tim);
    printf(" %24s\n", time_f);
}
