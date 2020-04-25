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
make_EHelper(jmp);
