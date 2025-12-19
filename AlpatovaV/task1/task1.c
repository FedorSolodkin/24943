#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/resource.h>

extern char **environ;

int main(int argc, char *argv[])
{
   char options[] = ":ispuU:cC:dvV:";  /* valid options */
   int c, invalid = 0;

   printf("argc equals %d\n", argc);
   while ((c = getopt(argc, argv, options)) != EOF) {
      switch(c) {
      case 'i':
         printf("UID: %d\n", getuid());
         printf("EUID: %d\n", geteuid());
         printf("GID: %d\n", getgid());
         printf("EGID: %d\n", getegid());
         break;
      case 's':
         {
         if (setpgid(0, 0) == -1){
            perror("setpgid failed");
         }else{
            printf("Process became group leader\n");
         }
         }
         break;
      case 'p':
         {
         printf("%d\n", getpid());
         printf("%d\n", getppid());
         printf("%d\n", getppid());
         }
         break;
      case 'u':
         {
         struct rlimit rlim;
         if (getrlimit(RLIMIT_FSIZE, &rlim) == 0) {
            printf("ulimit: %ld\n", (long)rlim.rlim_cur);
         } else {
            printf("getrlimit failed");
         }
         }
         break;
      case 'U':
         {
         struct rlimit rlim;
         long new_limit = atol(optarg);
         
         if (getrlimit(RLIMIT_FSIZE, &rlim) == 0) {
            rlim.rlim_cur = new_limit;
            if (setrlimit(RLIMIT_FSIZE, &rlim) == -1) {
               perror("setrlimit failed");
            } else {
               printf("ulimit set to: %ld\n", new_limit);
            }
         } else {
            perror("getrlimit failed");
         }
         }
         break;
      case 'c':
         {
         struct rlimit rlim;
         if (getrlimit(RLIMIT_CORE, &rlim) == 0) {
            printf("Core file size: %ld bytes\n", (long)rlim.rlim_cur);
         } else {
            perror("getrlimit failed");
         }
         }
         break;
      case 'C':
         {
            struct rlimit rlp;
            long new_rlim = atol(optarg);
            if (-1 == getrlimit(RLIMIT_CORE, &rlp)) {
               perror("failed to get the core-file cap limit\n");
               break;
            }
            rlp.rlim_cur = new_rlim;
            if (-1 == setrlimit(RLIMIT_CORE, &rlp)) {
               perror("failed to set the core-file cap limit\n");
            } else {
               printf("the core-file cap limit has been set successfully\n");
            }
         }
         break;
      case 'd':
         {
         char *pathname = getenv("PWD");
         if (pathname != NULL){
            printf("Current directory: %s", pathname);
         }else {
            printf("PWD environment variable not set\n");
         }
         }
                
         break;
      case 'v':
         {
         char **env = environ;
      
         while (*env != NULL) {
            printf("%s\n", *env);
            env++;
         }
         }
         break;
      case 'V':
         {
         char *name_value = optarg;
         if (putenv(name_value) != 0) {
            perror("failed to set the environmental variable");
         } else {
            printf("Environment variable set: %s\n", name_value);
         }
         }
         break;
      case ':':
         {
         printf("Option -%c requires an argument\n", optopt);
         invalid = 1;
      }
         break;
      case '?':
         {
         printf("Unrecognized option: -%c\n", optopt);
         invalid = 1;
      }
         break;
      }
   }

   return 0;
}


