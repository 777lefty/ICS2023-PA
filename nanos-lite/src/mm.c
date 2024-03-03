#include <memory.h>
#include <stdio.h>
#include <proc.h>

static void *pf = NULL;

void* new_page(size_t nr_page) {
  //printf("pf is %p\n",pf);
  void *old_pf=pf;
  pf+=nr_page*PGSIZE;
  //printf("new pf is %p\n", pf);
  return old_pf;
  //return NULL;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
  void *pg_start=new_page(n/PGSIZE);
  memset(pg_start, 0, n);
  return pg_start;
  //return NULL;
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

extern PCB *current;

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
	//assert(0);
  uintptr_t max_brk=current->max_brk;
  if(brk>=max_brk){
    uintptr_t max_brk_pn=max_brk>>12;
    uintptr_t brk_pn=brk>>12;
    void *page_start=new_page(brk_pn-max_brk_pn+2);
    for(int i=0;i<brk_pn-max_brk_pn+2;i++){
	    //assert(0);
      map(&current->as, (void *)(max_brk+i*0xfff), (void *)(page_start+i*0xfff), 1);
      //assert(0);
    }
    current->max_brk=(brk_pn+1)<<12;
  }
  //assert(0);
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
