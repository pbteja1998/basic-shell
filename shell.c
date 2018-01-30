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
#include "redirections.h"
#include "pipe.h"

#ifndef SHELL_H
#define SHELL_H
struct node{
  char process_name[100];
  pid_t pid;
  int job_number;
  struct node *next;
  struct node *prev;
};
#endif
static int flagss=0;
extern char *cwd;
extern char *hwd;
extern int n_bg_processes;
extern int flag ;
int BACK;
// char *PROCESS_NAME;
static struct node *bg,*head=NULL,*tail=NULL;

void insert(pid_t value,char *process_name){
  struct node *p;
  p = (struct node*)malloc(sizeof(struct node));
  p->pid = value;
  strcpy(p->process_name,process_name);
  if(head==NULL){
    head = p;
    p->job_number = 1;
    tail = head;
    return;
  }
  p->job_number = head->job_number+1;
  p->next = head;
  head->prev = p;
  p->prev = NULL;
  head = p;
}

void display_bg_processes(){
  struct node *p = tail;
  while(p!=NULL){
    printf("[%d] %s [ %d ] \n",p->job_number,p->process_name,p->pid);
    p=p->prev;
  }
}

void delete(struct node *p){

  struct node *q = p;
  while(q!=NULL){
      q->job_number--;
      q=q->prev;
  }

  if(p->prev==NULL){
    if(head==p){
      if(p->next!=NULL)
        head=p->next;
      else
        head=NULL;
    }
    free(p);
    return;
  }
  else if(p->next==NULL){
    if(p==head){
      head=NULL;
    }
    p->prev->next = NULL;
    free(p);
    return;
  }

  p->prev->next = p->next;
  p->next->prev = p->prev;
  free(p);
}

void check_for_bg_terminated_processes(){
  int status;
  struct node *p=head;
  while(p!=NULL){
    if(waitpid(p->pid,&status,WNOHANG)){ // STATE of child is changed
      if(WIFEXITED(status)){
        printf("%s with pid %d exited normally\n",p->process_name,p->pid);
        delete(p);
        n_bg_processes--;
      }
    }
    p = p->next;
  }
}

void delete_all_bg_processes(){
  struct node *p=head;
  while(p!=NULL){
    kill(p->pid,SIGKILL);
    delete(p);
    n_bg_processes--;
    p = p->next;
  }
}

void signal_handler(int sig_number){
}

void execute(char **argv,int background){
  pid_t  pid;
  int status;
  BACK = background;
  if ((pid = fork()) < 0) {
    perror("ERROR: forking child process failed");
    exit(1);
  }

  if (pid == 0) {
    // child process
    if(background) {
      fclose(stdin); // close child's stdin
      fopen("/dev/null", "r"); // open a new stdin that is always empty

      if(execvp(*argv,argv) == -1){
        fprintf (stderr, "unknown command: %s\n", argv[0]);
        exit(1);
      }
    }
    else {
      if(execvp(*argv,argv)==-1){
        fprintf (stderr, "unknown command: %s\n", argv[0]);
        exit(1);
      }
    }

  } else {
    //parent process
    if (background) {
      n_bg_processes++;
      insert(pid,argv[0]);
      printf("[%d] %s with pid %d started\n",head->job_number,argv[0],pid);

    }
    else {
      signal(SIGTSTP,signal_handler);
      // printf("%d\n",BACK);
        while(wait(&status)!=pid){
          // printf("%\n", );
        }
    }
  }
}

struct node *get_bg(int job_number){
  struct node *p=head;
  int i=1;
  while(p!=NULL){
    if(p->job_number==job_number){
      return p;
    }
    p = p->next;
  }

}

void sendsig(char *sub0,char *sub1) {
  struct node *p = get_bg(atoi(sub0));
  kill(p->pid,atoi(sub1));
  if(atoi(sub1)==9)
    delete(p);
}

int shell(char *arg1,char *arg2,int background){
  int j,i;
  char *input,S[100],*buffer,*s;
  buffer = (char*)malloc(sizeof(char)*1000);
  s = (char*)malloc(sizeof(char)*1000);
  input = (char*)malloc(sizeof(char)*1000);
  if(!strcmp(arg1,"cd")){
    if((cwd[strlen(cwd)-1]!='/')&&(strlen(cwd)!=0)) strcat(cwd,"/");
    input=arg2;
    int present_flag = flag;
    flag = 0;


    if(arg2[0]=='~') {
      arg2+=1;
      strcat(s,hwd);
      strcat(s,arg2);
      input = s;
    }

    int existence = access(input,F_OK);
    if(!existence){
      chdir(input);
      getcwd(cwd,100);
      for(j=0;j<strlen(hwd);j++){
        if(cwd[j]!=hwd[j])
        break;
      }

      if(j==strlen(hwd)){
        cwd+=strlen(hwd);
        flag=1;
      }
    }

    else{
        flag = present_flag;
       perror(arg2);
     }
  }

  else if(!strcmp(arg1,"pwd")){
    if(flag==1) cwd-=strlen(hwd);
    printf("%s\n",cwd);
    if(flag==1) cwd+=strlen(hwd);
  }

  else if(!strcmp(arg1,"listjobs")){
    display_bg_processes();
  }

  else if(!strcmp(arg1,"killallbg")){
    delete_all_bg_processes();
  }

  else if(!strcmp(arg1,"quit"))
    return 1; // break_flag = 1;

  else if(!strcmp(arg1,"echo")){

    if((arg2[0]=='\'')||(arg2[0]=='"')){
      arg2+=1;
      arg2[strlen(arg2)-1]='\0';
    }

      printf("%s\n",arg2);
  }

  else if(!strcmp(arg1,"fg")){
    struct node *p = get_bg(atoi(arg2));
    int status;
    waitpid(p->pid, &status, 0);
    // kill(p->pid,SIGKILL);
    delete(p);
  }

  else if(!strcmp(arg1,"sendsig")){
    char *str1,*saveptr1,*token,*sub[2];
    int i=0;
    for(str1=arg2;;str1=NULL){
      token = strtok_r(str1," ",&saveptr1);
      if(token==NULL) break;
      sub[i++] = token;
    }
    sendsig(sub[0],sub[1]);
  }

  else if(!strcmp(arg1,"pinfo")){
    sprintf(S,"/proc/%s/status",arg2);
    chdir(S);
    perror(arg2);
    int fd = open(S,O_RDONLY);
    read(fd,buffer,1000);
    char *str2,*token,*saveptr1;
    for (i=0, str2 = buffer; ;i++,str2 = NULL) {

      token = strtok_r(str2,"\n",&saveptr1);

      if (token == NULL) break;

      if((i==0)||(i==1)||(i==4)||(i==16))  // 0th line,1st line,4th line,16th line of proc/<pid>/stat
        printf("%s\n",token);
    }
    if(flag){
      cwd-=strlen(hwd);
      chdir(cwd);
      cwd+=strlen(hwd);
    }
    else chdir(cwd);
  }

  else{
    char *argv[100]={NULL};
    argv[0] = arg1;
    int k=1;

    char *str1,*saveptr1,*token;
    for(str1=arg2;;str1=NULL){
      token = strtok_r(str1," ",&saveptr1);
      if(token==NULL)
        break;
      argv[k++] = token;
    }

    argv[k] = NULL;
    execute(argv,background);
  }
  return 0;
}

int tokenise(char *command){

  int background = 0,i,j,executed=0;

  char *sub1,*str1,*str2,*saveptr1,*saveptr2,*token,*subtoken;

  for (j = 1, str1 = command; ; j++, str1 = NULL) {
    executed = 0;
    token = strtok_r(str1, ";", &saveptr1);
    background = 0;
    if (token == NULL) break;

    // int n_parameters = tok(token);
    for(int x=0;x<strlen(token);x++){
      if(token[x]=='|'){
        pipe_tokenise(token);
        executed=1;
        break;
      }
    }

    for(int x=0;x<strlen(token);x++){
      if(executed==1)
        break;
      if(token[x]=='>' || token[x]=='<'){
        tok(token);
        executed=1;
        break;
      }
    }
    if(executed)
      continue;
    // tok(token);
    if(token[strlen(token)-1]=='&'){
      background=1;
      token[strlen(token)-1] = '\0';
    }

    char sub2[1000] = {'\0'};

    for (i=0,str2 = token; ;i++, str2 = NULL) {
      subtoken = strtok_r(str2, " ", &saveptr2);
      if(subtoken == NULL)
      break;

      if(i==0)
        sub1=subtoken;                          //main command

      else{
        if(i>1){
          if(subtoken[0]=='-') subtoken+=1;    //If there are arguments in command
          else strcat(sub2," ");               //Else Leave it as it is
        }
        strcat(sub2,subtoken);                 //Combine all arguments if any
      }
    }

    if(shell(sub1,sub2,background))
      return 1;
  }
  return 0;
}
