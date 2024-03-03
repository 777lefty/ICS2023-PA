#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
	//assert(0);
  char *empty[] =  {NULL };
  int argc=args[0];
  char **argv=(char **)(args+1);
  char **envp=(char **)(args+argc+1);
  if(argc>0)printf("argv[0] is %s\n",argv[0]);
  environ = envp;
  exit(main(argc, argv, envp));
  assert(0);
}
