struct node{
  char *process_name;
  pid_t pid;
  int job_number;
  struct node *next;
  struct node *prev;
};
void insert(pid_t value,char *process_name);
void insert(pid_t value,char *process_name);
void delete(struct node *p);
void display_bg_processes();
void check_for_bg_terminated_processes();
void  execute(char **argv,int background);
void signal_handler(int sig_number);
struct node *get_bg(int job_number);
int shell(char *arg1,char *arg2,int background);
int tokenise(char *command);
void delete_all_bg_processes();
