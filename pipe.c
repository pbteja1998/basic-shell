#include "unistd.h"
#include "sys/types.h"
#include "sys/wait.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "signal.h"
#include "stdio.h"
#include "errno.h"
#include "string.h"
#include "stdlib.h"

int input_fd=0,output_fd=1;
char *fu(char *command){
  char *str1,*saveptr1,*token;
  for(str1=command;;str1=NULL){
    token = strtok_r(str1," ",&saveptr1);
    if(token==NULL) break;
    return token;
  }
}

char *tokenise_less_than(char *sub) {
  char *str1,*saveptr1,*token,*command;
  int i;
  for(i=0,str1=sub;;i++,str1=NULL){
    token = strtok_r(str1,"<",&saveptr1);
    if(token==NULL) break;
    token = fu(token);
    if(i==1){
      input_fd = open(token,O_RDONLY);
      dup2(input_fd,STDIN_FILENO);
      close(input_fd);
    }
    else command = token;
  }
  return command;
}


char *tokenise_greater_than(char *sub) {
  char *str1,*saveptr1,*token,*command;
  int i,fd;
  for(i=0,str1=sub;;i++,str1=NULL){
    token = strtok_r(str1,">",&saveptr1);
    if(token==NULL) break;
    if(i==1){
      token = fu(token);
      remove(token);
      output_fd = open(token,O_RDWR|O_CREAT,S_IRWXU);
      dup2(output_fd,STDOUT_FILENO);
      close(output_fd);
    }
    else command = token;
  }
  return command;
}


int check(char *sub){
  for(int i=0;i<strlen(sub);i++){
    if(sub[i]=='>')
    return 1;
    else if(sub[i]=='<')
    return 0;
  }
  return -1;
}


void execu(char **sub){
  int pfd[2],pid,fd_in=0,status;
  int i=0;
  int fd = dup(STDIN_FILENO);
  int fd1 = dup(STDOUT_FILENO);
  while(sub[i]!=NULL){
    pipe(pfd);
    pid = fork();
    if(pid==0){
//      printf("%s %d\n",sub[i],check(sub[i]));
      int c = check(sub[i]);
      if(c==0){
        sub[i]=tokenise_less_than(sub[i]);
      }
      else if(c==1){
        sub[i] = tokenise_greater_than(sub[i]);
        // printf("%s\n",sub[i]);
      }

      dup2(fd_in,STDIN_FILENO);

      if(sub[i+1]!=NULL){
        dup2(pfd[1],STDOUT_FILENO);
      }

      close(pfd[0]);

      char *str1,*saveptr1,*token,*argv[100]={NULL};
      int k=0;

      for(str1=sub[i];;str1=NULL){
        token = strtok_r(str1," ",&saveptr1);
        if(token==NULL)
        break;
        argv[k++] = token;
      }
      argv[k]=NULL;
      execvp(argv[0],argv);
    }
    else{

      while(wait(&status)!=pid);
      close(pfd[1]);
      fd_in = pfd[0];
      i++;

    }
  }

}

void pipe_tokenise(char *command){
  char *str1,*saveptr1,*token,*sub[100]={NULL};
  int i = 0;
  for(str1=command;;str1=NULL){
    token = strtok_r(str1,"|",&saveptr1);
    if(token==NULL)
      break;

    sub[i++] = token;
  }
  execu(sub);
}
