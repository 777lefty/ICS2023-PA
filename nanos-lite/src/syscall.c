#include <common.h>
#include "syscall.h"
#include <proc.h>

//#define CONFIG_STRACE true
#ifndef CONFIG_STRACE
#define CONFIG_STRACE false
#endif

int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
int fs_close(int fd);
void naive_uload(PCB *pcb, const char *filename);
void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]);
void switch_boot_pcb();

void sys_yield(Context *c){
  yield();
  c->GPRx=0;
}

void sys_exit(Context *c){
  halt(c->GPR1);
  //naive_uload(NULL, "/bin/nterm");
  //context_uload(current, "/bin/nterm", NULL, NULL);
  //assert(strcmp(argv[0], "pal")!=0);
  //switch_boot_pcb();
  //assert(strcmp(argv[0], "pal")!=0);
  //yield(); 
  c->GPRx=0;
}

void sys_write(Context* c){
  int fd=(int)(c->GPR2);
  void *buf=(void *)(c->GPR3);
  size_t count=(size_t)(c->GPR4);
  c->GPRx=fs_write(fd, buf, count);  
}

void sys_read(Context *c){
  int fd=(int)(c->GPR2);
  void *buf=(void *)(c->GPR3);
  size_t len=(size_t)(c->GPR4);
  c->GPRx=fs_read(fd, buf, len);
}

void sys_open(Context *c){
  char *path=(char*)(c->GPR2);
  int flags=(int)(c->GPR3);
  int32_t  mode=(int32_t)(c->GPR4); 
  c->GPRx=fs_open(path, flags, mode);
}

void sys_close(Context *c){
  int fd=(int)(c->GPR2);
  c->GPRx=fs_close(fd);
}

void sys_lseek(Context *c){
  int fd=(int)(c->GPR2);
  size_t offset=(size_t)(c->GPR3);
  int whence=(int)(c->GPR4);
  c->GPRx=fs_lseek(fd, offset, whence);
}

int mm_brk(uintptr_t brk);

void sys_brk(Context *c){
  c->GPRx=mm_brk((uintptr_t)(c->GPR2));
}

void sys_gettimeofday(Context *c){
  struct timeval *tv=(struct timeval*)c->GPR2;
  size_t t=io_read(AM_TIMER_UPTIME).us;
  tv->tv_sec=t/1000000;
  tv->tv_usec=t%1000000;
  c->GPRx=0; 
}

void sys_execve(Context *c){
  const char *fname=(const char *)c->GPR2;
  char *const *argv=(char *const *)c->GPR3;
  char *const *envp=(char *const *)c->GPR4;
  //printf("%s\n",argv[0]);
  if(fs_open(fname, 0, 0)<0){
    c->GPRx=-2;
  }
  else{
    context_uload(current, fname, argv, envp);
    //assert(strcmp(argv[0], "pal")!=0);
    switch_boot_pcb();
    //assert(strcmp(argv[0], "pal")!=0);
    yield();
    //naive_uload(NULL, fname);
    c->GPRx=0;
  }  
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
    case SYS_exit : sys_exit(c);if(CONFIG_STRACE){printf("system called exit\n");} break;
    case SYS_yield : sys_yield(c);if(CONFIG_STRACE){printf("system called yield\n");} break;
    case SYS_open : sys_open(c);if(CONFIG_STRACE){printf("system called open\n");} break;
    case SYS_write : sys_write(c);if(CONFIG_STRACE){printf("system called write\n");} break;
    case SYS_read : sys_read(c);if(CONFIG_STRACE){printf("system called read\n");} break;
    case SYS_close : sys_close(c);if(CONFIG_STRACE){printf("system called close\n");} break;
    case SYS_lseek : sys_lseek(c);if(CONFIG_STRACE){printf("system called lseek\n");} break;
    case SYS_brk : sys_brk(c);if(CONFIG_STRACE){printf("system called brk\n");} break;
    case SYS_gettimeofday : sys_gettimeofday(c);if(CONFIG_STRACE){printf("system called gettimeofday\n");} break;
    case SYS_execve : sys_execve(c);if(CONFIG_STRACE){printf("system called execve\n");} break; 
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
