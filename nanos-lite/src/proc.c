#include <proc.h>

#define MAX_NR_PROC 4

void naive_uload(PCB *pcb, const char *filename);
void context_kload(PCB *pcb, void (*entry)(void *), void *arg);
//void context_uload(PCB *pcb, const char *filename);
void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]); 
static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
//	assert(0);
  int j = 1, i = 1;
  while (1) {
    if(1){
      Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
      //j = 1;
      i ++;
    }
      j ++;
      //assert(0);
    yield();
    //assert(0);
  }
}

void init_proc() {
  char *const argv[]={"--skip", NULL};
  context_kload(&pcb[0], hello_fun, "A");
  //context_kload(&pcb[1], hello_fun, "B");
  context_uload(&pcb[1], "/bin/pal", argv, NULL);
  //context_uload(&pcb[0], "/bin/hello", NULL, NULL);
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  //naive_uload(NULL,"/bin/nterm"); 

}

Context* schedule(Context *prev) {
  current->cp=prev;
  current = (current == &pcb[0]) ? &pcb[1] : &pcb[0];
  return current->cp;
  //return NULL;
}
