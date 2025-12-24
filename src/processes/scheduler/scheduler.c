#include "scheduler.h"

#include "../process.h"
#include "assert.h"

#include <stddef.h>

typedef struct SchedulerNode {
  Process process;
  struct SchedulerNode* next;
  struct SchedulerNode* prev;
} SchedulerNode;

void* scheduler_start          = NULL;
void* scheduler_front          = NULL;
void* scheduler_end            = NULL;
void* scheduler_free_list      = NULL;
SchedulerNode* current_process = NULL;

static SchedulerNode* alloc_process(void) {
  void* process;
  if (scheduler_free_list) {
    process             = scheduler_free_list;
    scheduler_free_list = *(void**) scheduler_free_list;
  } else {
    if ((size_t) scheduler_front >= (size_t) scheduler_end)
      kernel_panic("Too many processes.\n");
    process         = scheduler_front;
    scheduler_front = ((SchedulerNode*) scheduler_front) + 1;
  }
  return process;
}

static void free_process(Process* process) {
  *(void**) process   = scheduler_free_list;
  scheduler_free_list = process;
}

void init_scheduler(void) {
  scheduler_start = malloc_immortal(PID_MAX * sizeof(Process), 8);
  scheduler_front = scheduler_start;
  scheduler_end   = (uint8_t*) scheduler_front + PID_MAX * sizeof(Process);
}

// fills in pid
Process* schedule_process(void) {
  SchedulerNode* node = alloc_process();
  if (current_process) { // insert node right behind current node
    node->prev                  = current_process->prev;
    node->next                  = current_process;
    current_process->prev->next = node;
    current_process->prev       = node;
  } else { // make new cyclic list from node
    node->prev      = node;
    node->next      = node;
    current_process = node;
  }
  node->process.pid = ((size_t) node - (size_t) scheduler_start) / sizeof(SchedulerNode);
  return &node->process;
}

Process* scheduler_next_process(void) {
  current_process = current_process->next;
  return &current_process->process;
}

Process* scheduler_current_process(void) {
  return &current_process->process;
}
