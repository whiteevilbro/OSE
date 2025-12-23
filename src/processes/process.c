#include "process.h"

#include "./scheduler/scheduler.h"
#include "memmgnt.h"
#include "utils.h"

#include <stddef.h>
#include <stdint.h>

extern noret restore_context(IntraContext* ctx);

void enqueue_process(void (*func)(void), const int argc, const char* argv[]) {
  Process* process    = schedule_process();
  process->executable = func;
  process->argc       = argc;
  process->argv       = argv;

  process->pdt = NULL;
}

noret switch_process(void) {
  Process* process = scheduler_next_process();
  if (!process->pdt) {
    process->pdt = create_VAS();

    // //todo: somehow make it less hardcoded? maybe function that creates VA should fill in cs:eip & ss:esp ?
    process->ctx = (IntraContext){
        .context = {
            .edi        = 0,
            .esi        = 0,
            .ebp        = 0,
            .esp        = 0,
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
            .eip        = (size_t) process->executable,
            .cs         = APP_CODE_SEGMENT,
            .eflags     = 0x202},
        .ss  = APP_DATA_SEGMENT,
        .esp = 0x800000};
  }
  enable_paging(process->pdt);
  restore_context(&process->ctx);
}
