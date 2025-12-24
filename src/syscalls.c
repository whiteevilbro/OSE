#include "syscalls.h"

#include "interrupts.h"
#include "paging.h"
#include "processes/process.h"
#include "processes/scheduler/scheduler.h"
#include "utils.h"

#include <stdio.h>

extern int param;

static void exit(const Context* const ctx) {
  //TODO:FIXME
  disable_paging();
  Process* process = scheduler_current_process();
  free_VAS(process->pdt);
  process->pdt = NULL;
  param++;
  printf("%p\n", process->lowest_stack_page);
  switch_process();
}

void register_syscalls(void) {
  set_interrupt_handler(0x31, INTERRUPT_GATE, USER_PL, exit);
}
