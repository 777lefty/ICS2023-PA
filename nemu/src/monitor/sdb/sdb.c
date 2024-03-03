/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/
#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include "watchpoint.h"
#include <memory/vaddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state=NEMU_QUIT;	
  return -1;
}

static int cmd_si(char *args) {
  int steps=0;
  if(args==NULL){
  //If the argument "N" isn't provided
    steps=1;
  }
  else{
  //else
    char start[2]={args[0],args[1]};
    if(strcmp(start,"0x")==0||strcmp(start,"0X")==0)sscanf(args+2,"%x",&steps);
    else sscanf(args,"%d",&steps);
  }
  for(int i=0;i<steps;i++)cpu_exec(1);
  return 0;
}

static int cmd_info(char *args) {
  if(strcmp(args,"r")==0){
    isa_reg_display();
  }
  else if(strcmp(args,"w")==0){
    printf("Num\tWhat\t\tOld\t\t\t\tNew\n");
    for(int i=0;i<NR_WP;i++){
      if(wp_pool[i].work==false){continue;}
      else{
	bool suc=true;      
        word_t new=expr(wp_pool[i].str,&suc);	
        printf("%d\t%s\t\t0x%08x %d\t\t0x%08x %d\n",i,wp_pool[i].str,wp_pool[i].val,wp_pool[i].val,new,new);
      }
    }
  }
  return 0;
}

static int cmd_x(char *args) {
  int cmd_x_len=0;
  vaddr_t cmd_x_saddr;
  char *cmd_x_N=strtok(args, " ");
  sscanf(cmd_x_N,"%d",&cmd_x_len);
  char *cmd_x_EXPR=strtok(NULL, " ");
  bool suc=true;
  cmd_x_saddr=expr(cmd_x_EXPR,&suc);
  for(int i=0;i<cmd_x_len;i++){
   printf("0x%x: 0x%08x\n",cmd_x_saddr+4*i,vaddr_read(cmd_x_saddr+4*i,4));
  }
  return 0;
}

static int cmd_p(char *args) {
  bool suc=true;
  word_t ret=expr(args,&suc);
  printf("EXPR=0x%08x\nEXPR=%u\nEXPR=%d\n",ret,ret,ret);
  return 0;
}

static int cmd_w(char *args) {
  WP *watch=new_wp();
  bool suc=true;
  sscanf(args,"%s",watch->str);
  watch->val=expr(args,&suc);
  //printf("%d\n",watch==&wp_pool[31]?1:0);
  //printf("%s %d %d %p\n",watch->str,watch->val,watch->work?1:0,watch);
  //for(int i=0;i<32;i++)printf("%p\n",wp_pool+i);
  return 0;
}

static int cmd_d(char *args) {
  int no=0;
  sscanf(args,"%d",&no);
  free_wp(&wp_pool[no]);
  return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  {"si", "Pasue the program after execute N instructions in single steps, if N is not provided, the default value is 1",cmd_si },
  {"info", "Print the states of registers or information of the watchpoints",cmd_info },
  {"x", "Solve the value of expression, use it as the starting memory address and output the following consecutive N Qbytes",cmd_x},
  {"p", "Solve the value of EXPR",cmd_p},
  {"w", "Set a watchpoint to pause the program when the value of EXPR changes",cmd_w},
  {"d", "Delete the number N watchpoint",cmd_d}
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
