#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

#define MSI 0x80000003
#define INT 0x80000007

static Context* (*user_handler)(Event, Context*) = NULL;
void __am_get_cur_as(Context *c);
void __am_switch(Context *c);

Context* __am_irq_handle(Context *c) {
  __am_get_cur_as(c);
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case MSI: if(c->GPR1==-1)ev.event = EVENT_YIELD;
	        else ev.event = EVENT_SYSCALL;
	        break;
      case INT: ev.event = EVENT_IRQ_TIMER; break;
      default: printf("c->mcause=%d\n",c->mcause);ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }
  __am_switch(c);
  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  Context *context=(Context *)(kstack.end-sizeof(Context)-4);
  context->mepc=(uintptr_t)entry;
  context->mstatus = 0x1800 | 0x80;
  context->gpr[10]=(uintptr_t)arg;
  context->pdir=NULL;
  context->np=0;
  context->gpr[2]=(uintptr_t)context;
  return context;
  //return NULL;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
