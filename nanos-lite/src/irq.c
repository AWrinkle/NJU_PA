#include "common.h"
#include "syscall.h"

extern _RegSet* do_syscall(_RegSet *r);
extern _RegSet* schedule(_RegSet *prev);

static _RegSet* do_event(_Event e, _RegSet* r) {
  printf("%d",e.event);
  switch (e.event) {
    case _EVENT_SYSCALL:
       return do_syscall(r);
    case _EVENT_TRAP: return schedule(r);
      //printf("receive an event trao!!!");break;
    case _EVENT_IRQ_TIME:/*Log("_EVENT_IRQ_TIME!!");*/return schedule(r);
    default: panic("Unhandled event ID = %d", e.event);
  }

  return NULL;
}

void init_irq(void) {
  _asye_init(do_event);
}
