#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  //TODO();
  //依次将EFLAGS，CS，EIP寄存器的值压入堆栈
  memcpy(&t1,&cpu.eflags,sizeof(cpu.eflags));
  rtl_li(&t0,t1);//*t0 = t1
  rtl_push(&t0);
  rtl_push(&cpu.cs);
  rtl_li(&t0,ret_addr);
  rtl_push(&t0);
  //从IDTR中读出IDT的首地址
  vaddr_t gate_addr=cpu.idtr.base+NO*sizeof(GateDesc);
  printf("idtr.base=0x%x\n",cpu.idtr.base);
  printf("idtr.limit=0x%x\n",cpu.idtr.limit);
  printf("gate_addr=0x%x\n",gate_addr);
  assert(gate_addr<=cpu.idtr.base+cpu.idtr.limit);
  //根据异常（中断）号在IDT中进行索引，找到一个门描述符
  //将门描述符中的offset域组合成目标地址
  uint32_t off_15_0=vaddr_read(gate_addr,2);
  uint32_t off_32_16=vaddr_read(gate_addr+sizeof(GateDesc)-2,2);
  uint32_t target_addr=(off_32_16<<16)+off_15_0;
#ifdef DEBUG
  Log("target_addr=0x%x",target_addr);
#endif

  //跳转到目标地址
  decoding.is_jmp=1;
  decoding.jmp_eip=target_addr;
}

void dev_raise_intr() {
}
