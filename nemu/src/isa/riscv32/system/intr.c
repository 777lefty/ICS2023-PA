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

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
#ifdef CONFIG_ETRACE
  printf("0x%08x: mcause = 0x%08x\n", epc, NO);
  log_write("0x%08x: mcause = 0x%08x\n", epc, NO);
#endif
  cpu.csr.mcause=NO;
  cpu.csr.mepc=epc;
  cpu.csr.mstatus.m.MIE=cpu.csr.mstatus.m.MPIE;
  cpu.csr.mstatus.m.MIE=0;
  return cpu.csr.mtvec;
}

#define IRQ_TIMER 0x80000007

word_t isa_query_intr() {
  if(cpu.csr.mstatus.m.MIE&&cpu.INTR){
    cpu.INTR=false;
    return IRQ_TIMER;
  }
  return INTR_EMPTY;
}
