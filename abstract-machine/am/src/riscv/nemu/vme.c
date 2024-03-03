#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

#define VP_N1(addr) (((uintptr_t)addr>>12)&0x000003ff)
#define VP_N2(addr) (((uintptr_t)addr>>22)&0x000003ff)

#define PP_N(addr) ((uintptr_t)addr>>10)
#define PP(addr) (((uintptr_t)addr>>12)<<10)

#define ALIGN(x) ((uintptr_t)(x)&0xfffff000)
int mmm=0;
void map(AddrSpace *as, void *va, void *pa, int prot) {
	mmm++;
  va=(void *)ALIGN(va);
  pa=(void *)ALIGN(pa);
  PTE *table_base=(PTE *)(as->ptr+4*VP_N2(va)); 
  //assert(0);
  if(!(*table_base&PTE_V)){
    void *new_page=pgalloc_usr(PGSIZE);
    *table_base=(*table_base&0x3ff)|PP((PTE)new_page)|PTE_V;
  }
  //assert(0);
  PTE *table_item=(PTE *)(PP_N(*table_base)*4096+VP_N1(va)*4);
  *table_item=PP(pa)|PTE_V;
  //assert(0);
  //assert(mmm<=100000);
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context *context=(Context *)(kstack.end-sizeof(Context)-4);
  context->mepc=(uintptr_t)entry;
  context->mstatus = 0x1800 | 0x80;
  context->np=1;
  context->gpr[2]=(uintptr_t)context;
  context->pdir=as->ptr;
  return context;
  //return NULL;
}
