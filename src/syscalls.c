#include "syscalls.h"

#include "console.h"
#include "interrupts.h"
#include "paging.h"
#include "processes/process.h"
#include "processes/scheduler/scheduler.h"
#include "utils.h"

static void exit(const Context* const ctx) {
  //TODO:FIXME
  disable_paging();
  Process* process = scheduler_current_process();
  printf("Process %d exited with code %#0.2x\n", process->pid, ctx->eax);
  kill_process(process);
  switch_process();
}

static void print_char(const Context* const ctx) {
  Process* process = scheduler_current_process();
  cputc((int) ctx->eax, &process->console);
  cflush(&process->console);
}

void register_syscalls(void) {
  set_interrupt_handler(0x30, INTERRUPT_GATE, USER_PL, exit);
  set_interrupt_handler(0x31, INTERRUPT_GATE, USER_PL, print_char);
}
