#include "syscalls.h"

#include "interrupts.h"
#include "processes/process.h"
#include "processes/scheduler/scheduler.h"
#include "utils.h"

#include <stdio.h>

extern int param;

static void exit(const Context* const ctx) {
  //TODO:FIXME
  __asm__ __volatile__(
      "mov eax, cr0;"
      "and eax, ~0x80000000;"
      "mov cr0, eax;" ::: "eax");
  Process* process = scheduler_current_process();
  free_VAS(process->pdt);
  process->pdt = NULL;
  param++;
  printf("%d\n", (int) ctx->eax);
  switch_process();
}

void register_syscalls(void) {
  set_interrupt_handler(0x31, INTERRUPT_GATE, USER_PL, exit);
}
