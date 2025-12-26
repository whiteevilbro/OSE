#ifndef PROCESS_H_
#define PROCESS_H_

#include "console.h"
#include "interrupts.h"
#include "paging.h"
#include "utils.h"

#include <stddef.h>

#define PID_MAX 16

#pragma pack(push, 1)

typedef struct Process {
  PageDirectoryEntry* pdt;
  Context ctx;

  size_t pid;               // for now it could be calculated from address. May change later
  void (*executable)(void); // some day this will be FILE* or /path/

  // technically, there's no need to store this fields explicitly, BUT i'll need to restart process, so here we are
  int argc;
  const char** argv;

  Console console;
  void* lowest_stack_page;
} Process;

#pragma pack(pop)

void init_scheduler(void);
void enqueue_process(void (*func)(void), const int argc, const char* argv[], Console* console);
noret switch_process(void);
noret save_and_switch_process(const Context* const ctx);
void kill_process(Process* process);


#endif
