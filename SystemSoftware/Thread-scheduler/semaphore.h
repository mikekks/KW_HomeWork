#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__
#include "thread.h"

#define MAX_SEMAPHORE_NUM (100)
#define SEM_NAME_MAX (100)

typedef struct _Semaphore {
    int count;
    ThreadQueue waitingQueue;
} Semaphore;

typedef struct _SemaphoreTblEnt {
    char name[SEM_NAME_MAX];
    BOOL bUsed;
    Semaphore *pSemaphore;
} SemaphoreTblEnt;

extern SemaphoreTblEnt pSemaphoreTblEnt[MAX_SEMAPHORE_NUM];

int thread_sem_open(char *name, int count);
int thread_sem_wait(int semid);
int thread_sem_post(int semid);
int thread_sem_close(int semid);

#endif // __SEMAPHORE_H__
