#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#define MAXLINE 2048
#define QUIT_MSG "exit"
#define DELIM_CHARS "|"

char cmdline[MAXLINE];
char* arg[100];
int fd_input;
int fd_output;
char* input; //Input Stream
char* output; //Output Stream
char* output_append; //Output Stream
pid_t c_pid;
int origin_pid=0;
int origin_ppid=0;
int wstatus;

int err_msg(char* cmd,int err_no){

    if(err_no==13){//EPERM
        fprintf(stderr,"%s: Permission denied\n",cmd);
    }
    
    else if(err_no==2){//ENOENT
        fprintf(stderr,"%s: No such file or directory\n",cmd);
    }
    
    else if(err_no==20){//ENOTDIR
        fprintf(stderr,"%s: Not a directory\n",cmd);
    }
    else{
        fprintf(stderr,"%s: Error occurred: %d\n",cmd,err_no);
    }
    return err_no;
}

void changeDir(char** arg) {
	if (arg[1] == NULL) {
		chdir(getenv("HOME"));
	} else {
		if (chdir(arg[1]) == -1) {
			err_msg(arg[0],errno);
		}
	}
}

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

char* split_redir(char *str, const char *delim){ //arg + Stream
    char *p = strstr(str, delim);
    if (p == NULL) return NULL;
    *p = '\0';                
    return p + strlen(delim); 
}

int split_pipe(char** arg){ //split by pipe
    int arg_size=0;
    char *ret_ptr; 
    char *next_ptr; 

    ret_ptr = strtok_r(cmdline, DELIM_CHARS, &next_ptr); 
    while(ret_ptr) { 
        arg[arg_size++]=ret_ptr;
        ret_ptr = strtok_r(NULL, DELIM_CHARS, &next_ptr); 
        }

    arg[arg_size] = NULL;
    
    return arg_size;
}

char** split_arg(char* buf){ // split by space
    char ** res  = NULL;
    char *  p    = strtok (buf, " ");
    int n_spaces = 0;

    while (p) {
        res = realloc (res, sizeof (char*) * ++n_spaces);

        if (res == NULL)
            break;
        res[n_spaces-1] = p;
        p = strtok (NULL, " ");
    }

    res = realloc (res, sizeof (char*) * (n_spaces+1));
    res[n_spaces] = 0;

    return res;
}

int cmd_run(char** array){

    if((strcmp(array[0],"ls")==0)|(strcmp(array[0],"grep")==0)
    |(strcmp(array[0],"mv")==0)|(strcmp(array[0],"cp")==0)){
        char path[100];
        int res;
        sprintf(path,"/bin/%s",array[0]);
        c_pid = fork();
        if (c_pid == -1) {
            err_msg(array[0],errno);
            kill(getpid(),SIGKILL);
        }
        else if (c_pid == 0) {
            if(execv(path,array) == -1) { 
            err_msg(array[0],errno);
            kill(getpid(),SIGKILL);
            }
        }

        else{
            int ret;
            
            if (ret == -1) {
                err_msg(array[0],errno);
                kill(getpid(),SIGKILL);
            }
            if (waitpid(c_pid, &wstatus, WUNTRACED | WCONTINUED) == -1) {
                err_msg(array[0],errno);
                kill(getpid(),SIGKILL);
            }
            }

    }

    else if((strcmp(array[0],"man")==0)|(strcmp(array[0],"sort")==0)|(strcmp(array[0],"awk")==0)|(strcmp(array[0],"bc")==0)
    |(strcmp(array[0],"av")==0)|(strcmp(array[0],"strace")==0)|(strcmp(array[0],"which")==0)|(strcmp(array[0],"where")==0)){
        char path[100];
        int res;
        
        sprintf(path,"/usr/bin/%s",array[0]);
        c_pid = fork();
                
        if (c_pid == -1) {
            err_msg(array[0],errno);
            kill(getpid(),SIGKILL);
        }
        else if (c_pid == 0) {
            if(execv(path,array) == -1) { 
            err_msg(array[0],errno);
            kill(getpid(),SIGKILL);
            }
        }

        else{
            int ret;
            
            if (ret == -1) {
                err_msg(array[0],errno);
                kill(getpid(),SIGKILL);
            }
            if (waitpid(c_pid, &wstatus, WUNTRACED | WCONTINUED) == -1) {
                err_msg(array[0],errno);
                kill(getpid(),SIGKILL);
            }
        }
    }

    else if(strcmp(array[0],"cd")==0){
        changeDir(array);
    }

    else if(strcmp(array[0],"pwd")==0){
        char cwd[MAXLINE];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        }
    }

    else if(strcmp(array[0],"exit")==0){
        
        int exitno;
        if(array[1]!=NULL){
            exitno=atoi(array[1]);
        }
        else{
            exitno=0;
        }
        fprintf(stderr,"exit %d\n",exitno);
        exit(exitno);
    }

    else if((strncmp(array[0],"./",2)==0)||(strncmp(array[0],"../",3)==0)){
            c_pid = fork();
                
        if (c_pid == -1) {
            err_msg(array[0],errno);
            kill(getpid(),SIGKILL);
        }
        else if (c_pid == 0) {
            if(execvp(array[0],array) == -1) { 
            err_msg(array[0],errno);
            kill(getpid(),SIGKILL);
            }
        }

        else{
            int ret;
            
            if (ret == -1) {
                err_msg(array[0],errno);
                kill(getpid(),SIGKILL);
            }
            if (waitpid(c_pid, &wstatus, WUNTRACED | WCONTINUED) == -1) {
                err_msg(array[0],errno);
                kill(getpid(),SIGKILL);
            }
        }
    }

    else{
        fprintf(stderr,"mysh: Command not found : %s\n",array[0]);
    }

    // return array;
    return 0;
}

int set_input(char *log_file){ 
    int fd;
    int stdin_save;
    stdin_save = dup(STDOUT_FILENO); /* save */ 
    if((fd = open(log_file, O_WRONLY | O_CREAT, 0666)) == -1) { 
        return 0; 
    } 
    dup2(fd, 0); 
    dup2(fd, 2); 
    close(fd); 
    return stdin_save; 
}

int set_input_rtn(char *log_file,int stdin_save){ 
    dup2(stdin_save,0); 
    dup2(stdin_save,2); 
    close(stdin_save); 
    return 0; 
}

int set_output(char *log_file){ 
    int fd;
    int stdout_save;
    stdout_save = dup(STDOUT_FILENO); /* save */
    if((fd = open(log_file, O_WRONLY | O_CREAT, 0666 | O_APPEND)) == -1) { 
        fprintf(stdout,"mysh: No such file"); 
        return 0; 
    } 
    dup2(fd, 1); 
    dup2(fd, 2); 
    close(fd); 

    return stdout_save; 
}

int set_output_append(char *log_file){ 
    int fd;
    if((fd = open(log_file, O_WRONLY | O_APPEND)) == -1) {
        fprintf(stdout,"mysh: No such file"); 
        return 0; 
    } 
    dup2(fd, 1); 
    dup2(fd, 2); 
    close(fd); 
    return 0; 
}

int set_output_rtn(char *log_file,int stdout_save){ 
    dup2(stdout_save,1); 
    dup2(stdout_save,2); 
    close(stdout_save); 
    return 1; 
}


int main(int argc, char** argv){
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, (void *)sig_handler);
        
    while(!feof(stdin)){
        fputs("mysh: ",stdout);
        int input_fd;
        int output_fd;

        fgets(cmdline,sizeof(cmdline),stdin);
        cmdline[strlen(cmdline)-1] = '\0';
        
        if(strlen(cmdline)==0){
            return 0;
        }
        else if(strcmp(cmdline,QUIT_MSG)==0)
                return 0;

        else{
            int arg_size=split_pipe(arg);
            input=NULL;
            output_append=NULL;
            output=NULL;
            
            //1) Input
            if(strstr(arg[0],"<")!=NULL){
                input=split_redir(arg[0],"<");
            }
            //2) Output_Append
            else if(strstr(arg[arg_size-1],">>")!=NULL){
                output_append = split_redir(arg[arg_size-1], ">>");
            }
            //2) Output
            else if(strstr(arg[arg_size-1],">")!=NULL){
                output = split_redir(arg[arg_size-1], ">");
            }

            for(int i=0;i<arg_size;i++){
                char** array=split_arg(arg[i]);

                if((i==0)&&(input!=NULL)){
                    input_fd=set_input(input);
                }
                if(i==arg_size-1){
                    if(output_append!=NULL)
                        output_fd=set_output(output_append);
                    else if(output!=NULL)
                        output_fd=set_output(output);
                }
                    
                cmd_run(array);
                if(input!=NULL){
                    set_input_rtn(output,input_fd);
                }
            }
            if((output_append!=NULL)||(output!=NULL)){
                set_output_rtn(output,output_fd);
            }
        }
    }
}