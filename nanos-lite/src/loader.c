#include "common.h"
#include "fs.h"

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();
extern void _map(_Protect *p,void *va,void *pa);
extern void* new_page(void);
//extern uint8_t ramdisk_start;
//extern uint8_t ramdisk_end;
#define DEFAULT_ENTRY ((void *)0x8048000)

uintptr_t loader(_Protect *as, const char *filename) {
  //TODO();
  //size_t size = get_ramdisk_size();
  //ramdisk_read(DEFAULT_ENTRY,0,size);
  int fd=fs_open(filename,0,0);
  //Log("filename=%s,fd=%d",filename,fd);
  //fs_read(fd,DEFAULT_ENTRY,fs_filesz(fd));
  //fs_close(fd);
   
  int len=fs_filesz(fd);
  void* pa;
  void* va;
  Log("filename=%s,fd=%d,size:%d\n",filename,fd,len);
  void* end=DEFAULT_ENTRY+len;
  for(va=DEFAULT_ENTRY;va<end;va+=PGSIZE)
  {
    pa=new_page();
    _map(as,va,pa);
    fs_read(fd,pa,(pa-va)<PGSIZE?(end-va):PGSIZE);
  }

  return (uintptr_t)DEFAULT_ENTRY;
}
/*
uintptr_t loader(_Protect *as, const char *filename) {
  size_t size = get_ramdisk_size();
  void * buff = NULL;
  ramdisk_read(buff,0,size); 
  memcpy(DEFAULT_ENTRY,buff,size); //之前误用memset
  //后来才想起来，ramdisk_read已经memcpy了，上一句无用功
  return (uintptr_t)DEFAULT_ENTRY;

  int fd = fs_open(filename, 0, 0);
  int bytes = fs_filesz(fd); //出错在之前为size_t

  Log("Load [%d] %s with size: %d", fd, filename, bytes);

  void *pa,*va = DEFAULT_ENTRY;
  while(bytes>0){
  	pa = new_page(); //申请空闲物理页
  	_map(as, va, pa);//该物理页映射到用户程序虚拟地址空间
  	fs_read(fd, pa, PGSIZE);  //读一页文件到该物理页

  	va += PGSIZE;
  	bytes -= PGSIZE;
  }
  //fs_read(fd,DEFAULT_ENTRY,bytes);
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}
*/
