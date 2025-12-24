#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "../process.h"

void init_scheduler(void);
Process* schedule_process(void);
Process* scheduler_next_process(void);
Process* scheduler_current_process(void);

#endif
