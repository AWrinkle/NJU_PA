#include "nemu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */



uint32_t paddr_read(paddr_t addr, int len) {
  //printf("%X  ",addr);
  //addr=addr%PMEM_SIZE;
  int r=is_mmio(addr);
  //printf("%X\n",addr);
  //printf("%d\n",r);
  if(r==-1)
          return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  else
          return mmio_read(addr,len,r);    
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int r=is_mmio(addr);
  if(r==-1)
           memcpy(guest_to_host(addr), &data, len);
  else
           mmio_write(addr,len,data,r);
}



/*

paddr_t page_translate(vaddr_t addr,bool is_write)
{
  PDE pde, *pgdir;
  PTE pte, *pgtable;
  paddr_t paddr=addr;
  if (cpu.cr0.protect_enable && cpu.cr0.paging) 
  {
    pgdir=(PDE*)(intptr_t)(cpu.cr3.page_directory_base<<12);
    pde.val=paddr_read((intptr_t)&pgdir[(addr>>22)&0x3ff],4);
    //printf("%d\n",pde.present);
    assert(pde.present);
    pde.accessed=1;
    
    pgtable=(PTE*)(intptr_t)(pde.page_frame<<12);
    pte.val=paddr_read((intptr_t)&pgtable[(addr>>12)&0x3ff],4);
    assert(pte.present);
    pte.accessed=1;
    pte.dirty=is_write?1:pte.dirty;
    paddr=(pte.page_frame<<12)|(addr&PAGE_MASK);
  }
  return paddr;
}

bool is_cross_boundry(vaddr_t addr,int len)
{
  bool result;
  result=(((addr+len-1)&~PAGE_MASK)!=(addr&~PAGE_MASK))?true:false;
  return result;
}


uint32_t vaddr_read(vaddr_t addr, int len) {
  paddr_t paddr;
  if(is_cross_boundry(addr,len))
  {
     union{
        uint8_t bytes[4];
        uint32_t dword;
     }data={0};
     for(int i=0;i<len;i++)
     {
        paddr=page_translate(addr+i,false);
        data.bytes[i]=paddr_read(paddr,1);
     }
     return data.dword;
  }
  else{
     paddr=page_translate(addr,false);
     return paddr_read(paddr,len);
  }

  //return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  //paddr_write(addr, len, data);
  paddr_t paddr;
  if(is_cross_boundry(addr,len))
  {
     for(int i=0;i<len;i++)
     {
        paddr=page_translate(addr+i,true);
        paddr_write(paddr,1,data);
        data>>=8;
     }
  }
  else
  {
     paddr=page_translate(addr,true);
     return paddr_write(paddr,len,data);
  }
}

*/


// 从x86.h里抄过来的
// +--------10------+-------10-------+---------12----------+
// | Page Directory |   Page Table   | Offset within Page  |
// |      Index     |      Index     |                     |
// +----------------+----------------+---------------------+
//  \--- PDX(va) --/ \--- PTX(va) --/\------ OFF(va) ------/
#define PDX(va)     (((uint32_t)(va) >> 22) & 0x3ff)
#define PTX(va)     (((uint32_t)(va) >> 12) & 0x3ff)
#define OFF(va)     ((uint32_t)(va) & 0xfff)
#define PTE_ADDR(pte)   ((uint32_t)(pte) & ~0xfff)
paddr_t page_translate(vaddr_t addr, bool w1r0) {
    Log("before:addr=%x",addr);
    //aka page_walk
    PDE pde, *pgdir;
    PTE pte, *pgtab;
    // 只有进入保护模式并开启分页机制后才会进行页级地址转换。。。。。。。。。。
    if (cpu.cr0.protect_enable && cpu.cr0.paging) {
	    pgdir = (PDE *)(PTE_ADDR(cpu.cr3.val)); //cr3存放20位的基址作为页目录入口
	    pde.val = paddr_read((paddr_t)&pgdir[PDX(addr)], 4);
	    assert(pde.present);
	    pde.accessed = 1;

	    pgtab = (PTE *)(PTE_ADDR(pde.val));  //页目录存放20位的基址作为页表入口
	    pte.val = paddr_read((paddr_t)&pgtab[PTX(addr)], 4);
	    assert(pte.present);
	    pte.accessed = 1;
	    pte.dirty = w1r0 ? 1 : pte.dirty; //写则置脏位

	    //pte高20位和线性地址低12位拼接成真实地址
            Log("cr3=0x%x,cr0=0x%x",cpu.cr3.val,cpu.cr0.val);
            Log("after=%x",PTE_ADDR(pte.val) | OFF(addr));
	    return PTE_ADDR(pte.val) | OFF(addr); 
	}
    return addr;
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  //PAGE_MASK = 0xfff
  if ((((addr) + (len) - 1) & ~PAGE_MASK) != ((addr) & ~PAGE_MASK)) {
	//data cross the page boundary
	uint32_t data = 0;
	for(int i=0;i<len;i++){
		paddr_t paddr = page_translate(addr + i, false);
		data += (paddr_read(paddr, 1))<<8*i;
	}
	return data;
	//assert(0);
  } else {
    paddr_t paddr = page_translate(addr, false);
    return paddr_read(paddr, len);
  }
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if ((((addr) + (len) - 1) & ~PAGE_MASK) != ((addr) & ~PAGE_MASK)) {
	//data cross the page boundary
	for(int i=0;i<len;i++){ //len 最大为4
		paddr_t paddr = page_translate(addr + i,true);
		paddr_write(paddr,1,data>>8*i);
	}
	//assert(0);
  } else {
    paddr_t paddr = page_translate(addr, true);
    paddr_write(paddr, len, data);
  }
}

