#ifndef __THREAD_H__
#define __THREAD_H__

#define _POSIX_C_SOURCE 199309L // RTC 사용하기 위해 필요

#include <linux/sched.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define TIMESLICE (2)
#define MAX_THREAD_NUM (64)  /* 생성 가능한 최대 Thread의 개수 */
#define STACK_SIZE 1024 * 64 // stack 사이즈

typedef int BOOL;
typedef int thread_t;
typedef struct _Thread Thread;
typedef void thread_attr_t;

typedef enum {
    THREAD_STATUS_RUN = 0,
    THREAD_STATUS_READY = 1,
    THREAD_STATUS_WAIT = 2,
    THREAD_STATUS_ZOMBIE = 3,
} ThreadStatus;

// running Queue, waiting Queue 의미
typedef struct _Queue {
    int queueCount;
    Thread *pHead;
    Thread *pTail;
} ThreadQueue;

// TCB 의미
typedef struct _Thread {
    int stackSize;
    void *stackAddr;
    ThreadStatus status;
    pid_t pid;
    double cpu_time;
    Thread *pPrev;
    Thread *pNext;
} Thread;

typedef struct _ThreadTblEnt {
    BOOL bUsed;
    Thread *pThread;
} ThreadTblEnt;

int thread_create(thread_t *thread, thread_attr_t *attr,
                  void *(*start_routine)(void *), void *arg);
int thread_suspend(thread_t tid);
int thread_cancel(thread_t tid);
int thread_resume(thread_t tid);
thread_t thread_self();
int thread_join(thread_t tid);
int thread_cputime(void);
void thread_exit(void);

extern ThreadQueue ReadyQueue;
extern ThreadQueue WaitingQueue;
extern ThreadTblEnt pThreadTbEnt[MAX_THREAD_NUM];
extern Thread *pCurrentThread;

#endif // __THREAD_H__
