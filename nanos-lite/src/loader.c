#include "common.h"

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();
#define DEFAULT_ENTRY ((void *)0x4000000)

uintptr_t loader(_Protect *as, const char *filename) {
  //TODO();
  size_t size = get_ramdisk_size();
  void * buff = NULL;
  ramdisk_read(buff,0,size); 
  memcpy(DEFAULT_ENTRY,buff,size);
  return (uintptr_t)DEFAULT_ENTRY;
}
