#include "common.h"
#include "fs.h"

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();
extern void _map(_Protect *p,void *va,void *pa);
extern void* new_page(void);
//extern uint8_t ramdisk_start;
//extern uint8_t ramdisk_end;
#define DEFAULT_ENTRY ((void *)0x400000)

uintptr_t loader(_Protect *as, const char *filename) {
  //TODO();
  //size_t size = get_ramdisk_size();
  //ramdisk_read(DEFAULT_ENTRY,0,size);
  int fd=fs_open(filename,0,0);
  //Log("filename=%s,fd=%d",filename,fd);
  //fs_read(fd,DEFAULT_ENTRY,fs_filesz(fd));
  //fs_close(fd);
   
  size_t len=fs_filesz(fd);
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
