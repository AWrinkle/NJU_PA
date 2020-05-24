#include "common.h"
#include "syscall.h"



static inline _RegSet* sys_none(_RegSet *r){
  SYSCALL_ARG1(r) = 1; //约定系统调用返回值存于此，即eax
  return NULL;
}

static inline _RegSet* sys_exit(_RegSet *r){
  //接受一个退出状态的参数，顺序ebx ecx edx
  _halt(SYSCALL_ARG2(r)); 
  return NULL;
}


int sys_write(int fd,void* buf,size_t len)
{
  if(fd==1||fd==2)
  {
     char c;
     for(int i=0;i<len;i++)
     {
        memcpy(&c,buf+i,1);
        _putc(c);
     }
     return len;
  }
  else
     panic("Unhandled fd =%d in sys_write",fd);
  return -1;
}


_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
    case SYS_none:
      return sys_none(r);
    case SYS_exit:
      return sys_exit(r);
    case SYS_write:
      SYSCALL_ARG1(r)=sys_write(a[1],(void*)a[2],a[3]);
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
