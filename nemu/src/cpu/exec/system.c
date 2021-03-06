#include "cpu/exec.h"

extern void raise_intr(uint8_t NO,vaddr_t ret_addr);

void diff_test_skip_qemu();
void diff_test_skip_nemu();

make_EHelper(lidt) {
  //TODO();
  t1=id_dest->val;
  rtl_lm(&t0,&t1,2);//*t0 = vaddr_read(*t1, 2);
  cpu.idtr.limit=t0;  
  
  t1=id_dest->val+2;
  rtl_lm(&t0,&t1,4);
  cpu.idtr.base=t0;

#ifdef DEBUG
  Log("idtr.limit=0x%x",cpu.idtr.limit);
  Log("idtr.base=0x%x",cpu.idtr.base);
#endif
  print_asm_template1(lidt);
}


make_EHelper(mov_r2cr) {
  //TODO();
  if(id_dest->reg==0)
    {
    cpu.cr0.val=reg_l(id_src->reg);
    //Log("cr0,eip=0x%x",cpu.eip);
    }
  else if(id_dest->reg==3)
    {
    cpu.cr3.val=reg_l(id_src->reg);
    //Log("id_src=0x%x",reg_l(id_src->reg));
    //Log("cr3,eip=0x%x",cpu.eip);
    //cpu.cr3.val=0x01d92000;
    }
  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  //TODO();
  if(id_src->reg==0)
    {
    reg_l(id_dest->reg)=cpu.cr0.val;
    //Log("cr0,eip=0x%x",cpu.eip);
    }
  else if(id_src->reg==3)
    {
    reg_l(id_dest->reg)=cpu.cr3.val;
    //Log("cr3,eip=0x%x",cpu.eip);
    }
  //Log("cr3=0x%x,cr0=0x%x,id_dest=0x%x",cpu.cr3.val,cpu.cr0.val,reg_l(id_dest->reg));
  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}




make_EHelper(int) {
  //TODO();
  uint8_t NO=id_dest->val & 0xff;
  raise_intr(NO,decoding.seq_eip);
  print_asm("int %s", id_dest->str);

#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}

make_EHelper(iret) {
  //TODO();
  rtl_pop(&cpu.eip);
  rtl_pop(&cpu.cs);
  rtl_pop(&t0);
  memcpy(&cpu.eflags,&t0,sizeof(cpu.eflags));
  
  decoding.jmp_eip=1;
  decoding.seq_eip=cpu.eip;

  print_asm("iret");
}

uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);

make_EHelper(in) {
  //TODO();
  rtl_li(&t0,pio_read(id_src->val,id_dest->width));
  operand_write(id_dest,&t0);

  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(out) {
  //TODO();
  pio_write(id_dest->val,id_src->width,id_src->val);
  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}
