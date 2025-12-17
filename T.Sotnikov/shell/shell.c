/* --- START OF FILE shell.c --- */

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include "shell.h"
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>
#include <string.h>

pid_t stopped_pid = 0;

char *infile, *outfile, *appfile;
struct command cmds[MAXCMDS];
char bkgrnd;

int main(int argc, char *argv[])
{
    int i;
    char line[1024];      /* allow large command lines */
    int ncmds;
    char prompt[50];      /* shell prompt */

    int input_fd = 0;
    int pipe_fd[2];
    pid_t pgid = 0; 

    /* PLACE SIGNAL CODE HERE */

    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    sprintf(prompt,"[%s] ", argv[0]);

    while (promptline(prompt, line, sizeof(line)) > 0) /* until eof */
    {
        // Очистка завершившихся фоновых процессов перед новым вводом
        while (waitpid(-1, NULL, WNOHANG) > 0);

        if ((ncmds = parseline(line)) <= 0)
        {
            continue;   /* read next line */
        }
        
#ifdef DEBUG
        {
            int i, j;
            for (i = 0; i < ncmds; i++)
            {
                for (j = 0; cmds[i].cmdargs[j] != (char *) NULL; j++)
                {
                    fprintf(stderr, "cmd[%d].cmdargs[%d] = %s\n",
                         i, j, cmds[i].cmdargs[j]);
                }
                fprintf(stderr, "cmds[%d].cmdflag = %o\n", i, cmds[i].cmdflag);
            }
        }
#endif

        input_fd = 0; 
        pgid = 0;

        for (i = 0; i < ncmds; i++)
        {
            if (cmds[i].cmdargs[0] == NULL) continue;

            if (strcmp(cmds[i].cmdargs[0], "exit") == 0) exit(0);

            /* FORK AND EXECUTE CODE GOES HERE */

            if (strcmp(cmds[i].cmdargs[0], "fg") == 0)
            {
                if (stopped_pid == 0)
                {
                    fprintf(stderr, "No jobs stopped.\n");
                    continue;
                }

                tcsetpgrp(0, stopped_pid);
                kill(stopped_pid, SIGCONT);
                
                int status;
                waitpid(stopped_pid, &status, WUNTRACED);

                if (WIFSTOPPED(status))
                {
                    fprintf(stderr, "\nProcess %d stopped again\n", stopped_pid);
                }
                else
                    stopped_pid = 0;

                tcsetpgrp(0, getpid());

                continue;
            }

            if (strcmp(cmds[i].cmdargs[0], "bg") == 0)
            {
                if (stopped_pid == 0)
                {
                    fprintf(stderr, "No jobs stopped.\n");
                    continue;
                }

                kill(stopped_pid, SIGCONT);
                printf("Process %d resumed in background\n", stopped_pid);
                continue;
            }

            if (cmds[i].cmdflag & OUTPIP)
            {
                if (pipe(pipe_fd) < 0)
                {
                    perror("pipe failed");
                    exit(1);
                }
            }

            pid_t pid = fork();

            if (pid < 0)
            {
                perror("Fork failed");
                exit(1);
            }

            if (pid == 0)
            {

                if (bkgrnd == 0)
                {
                    signal(SIGINT, SIG_DFL);
                    signal(SIGQUIT, SIG_DFL);
                    signal(SIGTSTP, SIG_DFL);
                }
                else
                {
                    signal(SIGINT, SIG_IGN);
                    signal(SIGQUIT, SIG_IGN);
                }

                if (pgid == 0) pgid = getpid();
                setpgid(0, pgid);

                if (bkgrnd == 0)
                    tcsetpgrp(0, pgid);

                if (input_fd != 0)
                {
                    dup2(input_fd, 0);
                    close(input_fd);
                }

                if (cmds[i].cmdflag & OUTPIP)
                {
                    close(pipe_fd[0]);
                    dup2(pipe_fd[1], 1);
                    close(pipe_fd[1]);
                }

                if (infile != NULL && i == 0)
                {
                    int fd_in = open(infile, O_RDONLY);
                    if (fd_in < 0) 
                    {
                        perror("open input");
                        exit(1);
                    }
                    if (dup2(fd_in, 0) < 0)
                    {
                        perror("dup2 input");
                        exit(1);
                    }
                    close(fd_in);
                }

                if (outfile != NULL && i == ncmds - 1)
                {
                    int fd_out = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                    if (fd_out < 0)
                    {
                        perror("open output");
                        exit(1);
                    }
                    if (dup2(fd_out, 1) < 0)
                    {
                        perror("dup2 output");
                        exit(1);
                    }
                    close(fd_out);
                }
                else if (appfile != NULL && i == ncmds - 1)
                {
                    int fd_app = open(appfile, O_WRONLY | O_CREAT | O_APPEND, 0666);
                    if (fd_app < 0)
                    {
                        perror("open append");
                        exit(1);
                    }
                    if (dup2(fd_app, 1) < 0)
                    {
                        perror("dup2 append");
                        exit(1);
                    }
                    close(fd_app);
                }

                execvp(cmds[i].cmdargs[0], cmds[i].cmdargs);
                perror("execvp");
                exit(1);
            }

            else 
            {
                if (pgid == 0) pgid = pid;
                setpgid(pid, pgid);

                if (input_fd != 0) 
                    close(input_fd);

                if (cmds[i].cmdflag & OUTPIP)
                {
                    close(pipe_fd[1]);
                    input_fd = pipe_fd[0];
                }

                if (bkgrnd == 0)
                {
                    if (cmds[i].cmdflag & OUTPIP)
                    {
                        continue; 
                    }

                    tcsetpgrp(0, pgid);

                    int status;
                    waitpid(pid, &status, WUNTRACED);

                    tcsetpgrp(0, getpid());

                    if (WIFSTOPPED(status))
                    {
                        stopped_pid = pid;
                        fprintf(stderr, "\nProcess %d stopped. Type \"fg\" to resume.\n", pid);
                    }
                }
                else 
                {
                    printf("Process %d started in background\n", pid);
                }
            }
        }
    } /* close while */
    return 0;
}

/* PLACE SIGNAL CODE HERE */
