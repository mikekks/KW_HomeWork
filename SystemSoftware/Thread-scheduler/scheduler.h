#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include "thread.h"

void RunScheduler(void);
void __ContextSwitch(int curpid, int newpid);

#endif
