#ifndef PROCESS_H_
#define PROCESS_H_

#include "interrupts.h"
#include "memmgnt.h"
#include "paging.h"
#include "utils.h"

#include <stddef.h>

#define PID_MAX 16

#pragma pack(push, 1)

typedef struct {
  Context context;
  uint32_t esp;
  Alignas(4) uint16_t ss;
} IntraContext;

typedef struct Process {
  PageDirectoryEntry* pdt;
  size_t pid;               // for now it could be calculated from address. May change later
  void (*executable)(void); // some day this will be FILE* or /path/

  // technically, there's no need to store this fields explicitly, BUT i'll need to restart process, so here we are
  int argc;
  const char** argv;

  IntraContext ctx;
} Process;

#pragma pack(pop)

void init_scheduler(void);
void enqueue_process(void (*func)(void), const int argc, const char* argv[]);
noret switch_process(void);


#endif
