#include "common.h"
#include "syscall.h"
#include "fs.h"
extern int mm_brk(uint32_t new_brk);

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
     Log("buffer:%s",(char*)buf);
     for(int i=0;i<len;i++)
     {
        memcpy(&c,buf+i,1);
        _putc(c);
     }
     return len;
  }
  if(fd>=3)
     return fs_write(fd,buf,len);
  Log("fd<=0");
  return -1;
}

static inline _RegSet* sys_brk(_RegSet *r) {
  //pa3 总是返回0，表示堆区大小总是调整成功
  //Log("!");
  //SYSCALL_ARG1(r) = 0;
  //pa4.1 真正的调整堆区
  SYSCALL_ARG1(r) = mm_brk(SYSCALL_ARG2(r));
  return NULL;
}

int sys_open(const char* filename)
{
  return fs_open(filename,0,0);
}

int sys_read(int fd,void* buf,size_t len)
{
  return fs_read(fd,buf,len);
}

int sys_close(int fd)
{
  return fs_close(fd);
}

int sys_lseek(int fd,off_t offset,int whence)
{
  return fs_lseek(fd,offset,whence);
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
    case SYS_brk:
      return sys_brk(r);
    case SYS_open:
      SYSCALL_ARG1(r)=sys_open((char*)a[1]);
      break;
    case SYS_read:
      SYSCALL_ARG1(r)=sys_read(a[1],(void*)a[2],a[3]);
      break;
    case SYS_close:
      SYSCALL_ARG1(r)=sys_close(a[1]);
      break;
    case SYS_lseek:
      SYSCALL_ARG1(r)=sys_lseek(a[1],a[2],a[3]);
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
