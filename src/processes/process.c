#include "process.h"

#include "./scheduler/scheduler.h"
#include "console.h"
#include "interrupts.h"
#include "memmgnt.h"
#include "paging.h"
#include "utils.h"

#include <stddef.h>
#include <stdint.h>

extern size_t current_quant_limit_millis;
extern size_t current_quant_limit_millis_fractions;

extern noret restore_context(Context* ctx);

static void fill_VAS(Process* process) {
  PageDirectoryEntry* pdt = process->pdt;
  const char** argv       = process->argv;

  for (size_t i = 0; i < 16; i++) {
    mount_page(pdt, (void*) ((i << 12) + 0x800000), (void*) ((size_t) process->executable + (i << 12)), true, true, false);
  }

  if (!process->argc)
    return;

  char** argv_page = malloc_page();

  if (process->argc > 1007) {
    process->argc = 1007;
  }
  for (size_t i = 0; i < (size_t) process->argc; i++) {
    void* page    = malloc_page();
    size_t length = strlen(process->argv[i]);
    memmove(page, argv[i], length + 1);
    void* address = (void*) (0x841000 + (i << 12));
    mount_page(pdt, address, page, true, true, true);
    argv_page[i] = address;
  }

  mount_page(pdt, (void*) 0x8400000, argv_page, true, true, true);
}

static void init_process(Process* process) {
  process->pdt = create_VAS();
  fill_VAS(process);

  process->lowest_stack_page = (void*) 0x7ff000;

  // //todo: somehow make it less hardcoded? maybe function that creates VA should fill in cs:eip & ss:esp ?
  process->ctx = (Context){
      .edi        = (size_t) 0x8400000,
      .esi        = (size_t) process->argc,
      .ebp        = 0,
      .pseudo_esp = 0,
      .ebx        = 0,
      .edx        = 0,
      .ecx        = 0,
      .eax        = 0,
      .gs         = APP_DATA_SEGMENT,
      .fs         = APP_DATA_SEGMENT,
      .es         = APP_DATA_SEGMENT,
      .ds         = APP_DATA_SEGMENT,
      .vector     = 0,
      .error_code = 0,
      .eip        = (size_t) 0x800000,
      .cs         = APP_CODE_SEGMENT,
      .eflags     = 0x202,
      .ss         = APP_DATA_SEGMENT,
      .esp        = 0x800000};

  clear_console(&process->console);
}

void enqueue_process(void (*func)(void), const int argc, const char* argv[], Console* console) {
  Process* process    = schedule_process();
  process->executable = func;
  process->argc       = argc;
  process->argv       = argv;

  process->console = *console;

  process->pdt = NULL;
}

noret save_and_switch_process(const Context* const ctx) {
  disable_paging();
  cli();
  Process* process = scheduler_current_process();
  process->ctx     = *ctx;
  switch_process();
}

noret switch_process(void) {
  Process* process = scheduler_next_process();
  if (!process->pdt) {
    init_process(process);
  }
  set_cr3(process->pdt);
  enable_paging();
  restore_context(&process->ctx);
}

void kill_process(Process* process) {
  free_VAS(process->pdt);
  cprintf(&process->console, "Process exited\n");
  unschedule_process(process);
}
