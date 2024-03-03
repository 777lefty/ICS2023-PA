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
#include <memory/vaddr.h>
#include <memory/paddr.h>

#define VA_OFFSET(addr) (addr&0x00000fff)
#define VPN_1(addr) ((addr>>12)&0x000003ff)
#define VPN_2(addr) ((addr>>22)&0x000003ff)

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  uint32_t satp_PPN=cpu.csr.satp&0x003fffff;

  paddr_t table_base_loc=satp_PPN*4096+VPN_2(vaddr)*4;
  uint32_t table_base=paddr_read(table_base_loc, sizeof(uint32_t));

  paddr_t table_item_loc=(table_base>>10)*4096+VPN_1(vaddr)*4;
  uint32_t table_item=paddr_read(table_item_loc, sizeof(uint32_t));

  paddr_t paddr=(table_item>>10)*4096+VA_OFFSET(vaddr);
  //assert(0);
  //assert(paddr==vaddr);
  return paddr;
  //return MEM_RET_FAIL;
}
