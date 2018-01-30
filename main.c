#include "pwd.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "errno.h"
#include "string.h"
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "shell.h"
#include "redirections.h"
#include "pipe.h"

char *cwd,*hwd;
int flag=1; //current_working_directory is child of ~ of this program
int n_bg_processes = 0;

int stdin_dup,stdout_dup;
char *pcName,*username;

void print() {
  if(flag) printf("%s@%s:~%s$ ",username,pcName,cwd);
  else printf("%s@%s:%s$ ",username,pcName,cwd);
}


int main() {

  printf("Welcome\n");

  struct passwd *pws = getpwuid(geteuid());
  pcName = (char*)malloc(sizeof(char)*1000);
  gethostname(pcName,1000);
  username = pws->pw_name;
  cwd = (char*)malloc(sizeof(char)*1000);   //current_working_directory
  hwd = (char*)malloc(sizeof(char)*1000);   //home working directory
  getcwd(cwd,1000);
  getcwd(hwd,1000);

  cwd+=strlen(hwd);
  int i,j;
  char c;
  stdin_dup = dup(STDIN_FILENO);
  stdout_dup = dup(STDOUT_FILENO);

  while(1){

    dup2(stdin_dup,STDIN_FILENO);
    dup2(stdout_dup,STDOUT_FILENO);
    // if(signal(SIGINT,signal_handler)==SIG_ERR)
		// 	perror("Signal not caught!!");
    // print();
    if(flag) printf("%s@%s:~%s$ ",username,pcName,cwd);
    else printf("%s@%s:%s$ ",username,pcName,cwd);

    stdin_dup = dup(STDIN_FILENO);
    stdout_dup = dup(STDOUT_FILENO);

    char command[100],*l;
    signal(SIGINT,signal_handler);
    // signal(SIGTSTP,signal_handler);

    // scanf("%99[^\n]s",command);
    // scanf("%c",&c);
    l = fgets(command,100, stdin);

    if(l==NULL){
      break;
    }
    command[strlen(command)-1]='\0';
    check_for_bg_terminated_processes();

    if(command[0]=='\0')
      continue;

    if(tokenise(command)){
      delete_all_bg_processes();
      break;
    }
  }
  return 0;
}
