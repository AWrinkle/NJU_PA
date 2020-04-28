#include "cpu/exec.h"

make_EHelper(mov);
make_EHelper(operand_size);
make_EHelper(inv);
make_EHelper(nemu_trap);
make_EHelper(call);
make_EHelper(push);
make_EHelper(pop);
make_EHelper(sub);
make_EHelper(xor);
make_EHelper(ret);

//PA2第二阶段
//add.c
make_EHelper(lea);
make_EHelper(and);
make_EHelper(nop);
make_EHelper(add);
make_EHelper(cmp);
make_EHelper(setcc);
make_EHelper(movsx);
make_EHelper(movzx);
make_EHelper(test);
make_EHelper(jcc);
//add-longlong.c
make_EHelper(adc);
make_EHelper(or);
//bit.c
make_EHelper(sar);
make_EHelper(shl);
make_EHelper(dec);
make_EHelper(jmp);//根据偏移量计算并跳转
//bubble-sort.c
make_EHelper(inc);
//fact.c
make_EHelper(imul1);
make_EHelper(imul2);
make_EHelper(imul3);
//goldbach.c
make_EHelper(cltd);
make_EHelper(idiv);
//hello-str.c
make_EHelper(jmp_rm);//根据寄存器取值并跳转
make_EHelper(leave);
make_EHelper(div);
//load-store.c
make_EHelper(not);
//mul-longlong.c
make_EHelper(mul);
//recursion.c
make_EHelper(call_rm);
//shift.c
make_EHelper(shr);
//sub-longlong.c
make_EHelper(sbb);
//串口
make_EHelper(in);
make_EHelper(out);
//coremark
make_EHelper(cwtl);
//microbench
make_EHelper(neg);
make_EHelper(rol);
