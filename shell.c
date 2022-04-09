#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define MAXLINE 2048
#define QUIT_MSG "exit"
#define DELIM_CHARS "|"

int origin_pid=0;
int origin_ppid=0;

void sig_handler(int signo){
    if (signo == SIGTSTP){
        int pid = getpid();
        int ppid = getppid();
        int pgid = getpgid(ppid);

        if(origin_pid==0)
            origin_pid=pid;

        if(origin_ppid==0)
            origin_ppid=ppid;

        if(origin_pid!=0){
            while (origin_ppid != ppid) {
                kill(pgid, SIGKILL);
            }
        }
    }
}

int main(int argc, char** argv){
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, (void *)sig_handler);

    return 0;
}