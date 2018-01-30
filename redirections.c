#include "unistd.h"
#include "sys/types.h"
#include "sys/wait.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "signal.h"
#include "stdio.h"
#include "errno.h"
#include "string.h"
#include "stdlib.h"
#include "shell.h"

char *remove_extra_spaces(char *command){
  char *str1,*saveptr1,*token;
  char *a = (char *)malloc(sizeof(char)*1000);
  int i;
  for(i=0,str1=command;;i++,str1=NULL){
    token = strtok_r(str1," ",&saveptr1);
    if(token==NULL)
      break;

    if(i>0)
      strcat(a," ");

    strcat(a,token);
  }
  return a;
}

void exec(int count,char *a,char *b,char *c){

  int fd,fd1;

  fd = open(b,O_RDWR|O_CREAT,S_IRWXU);
  if(fd==-1) perror("open");
  if(count>1) fd1 = open(c,O_RDWR|O_CREAT,S_IRWXU);
  if(fd1==-1) perror("open");

  if(count%2==0){
     if(dup2(fd,STDIN_FILENO)==-1) perror("dup2");
     if(count>1){
       close(fd1);
       remove(c);
       fd1 = open(c,O_RDWR|O_CREAT,S_IRWXU);
       dup2(fd1,STDOUT_FILENO);
    }
  }
  else{
    close(fd);
    remove(b);
    fd = open(b,O_RDWR|O_CREAT,S_IRWXU);
    if(dup2(fd,STDOUT_FILENO)==-1)
      perror("dup2");
    if(count>1)
     if(dup2(fd1,STDIN_FILENO)==-1)
      perror("dup2");
  }

  char *argv[100]={NULL};

  int k=0;

  char *str1,*saveptr1,*token;
  for(str1=a;;str1=NULL){
    token = strtok_r(str1," ",&saveptr1);
    if(token==NULL)
      break;
    argv[k++] = token;
  }

  argv[k] = NULL;
  execute(argv,0);
}

void tok(char *command){
  //0  <
  //1 >
  //2 < >
  //3 > <
  char *str1,*str2,*saveptr1,*saveptr2,*token,*subtoken,*sub[2]={NULL};
  int i = 0,j=0,k=0,a=0,count=-1;
  for(j=0,str1=command;;j++,str1=NULL){
    token = strtok_r(str1,">",&saveptr1);

    if(token==NULL) break;

    for(k=0,str2=token;;k++,str2=NULL){
      subtoken = strtok_r(str2,"<",&saveptr2);

      if(subtoken==NULL) break;
      sub[i++]=remove_extra_spaces(subtoken);    // removing extra spaces in subtoken and storing in an array

      if(j==1){                   // ">" exists for sure
        if(count==-1) count=1;    // If there is no symbol before this
        else count+=2;            // If there is already a symbol before this
      }

      if(k==1 && count<2){       // "<" exists for sure
        if(count==-1) count=0;   // If there is no symbol before this
        else count+=2;          // If there is already a symbol before this
      }
    }
  }
  exec(count,sub[0],sub[1],sub[2]);
}
