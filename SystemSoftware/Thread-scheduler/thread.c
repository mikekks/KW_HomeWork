#include "thread.h"
#include "init.h"
#include "scheduler.h"
#include <stdio.h>
#include <string.h>

ThreadQueue ReadyQueue;
ThreadQueue WaitingQueue;
ThreadTblEnt pThreadTbEnt[MAX_THREAD_NUM];
Thread *pCurrentThread; // Running 상태의 Thread를 가리키는 변수

// 구현해야 하는 함수들

void Init(void) {
    ReadyQueue.pHead = NULL;
    ReadyQueue.pTail = NULL;
    ReadyQueue.queueCount = 0;

    WaitingQueue.pHead = NULL;
    WaitingQueue.pTail = NULL;

    WaitingQueue.queueCount = 0;
    for (int i = 0; i < MAX_THREAD_NUM; i++)
        pThreadTbEnt[i].bUsed = 0;
}

int thread_create(thread_t *thread, thread_attr_t *attr,
                  void *(*start_routine)(void *), void *arg) {
    Thread *stack;
    stack = malloc(STACK_SIZE);
    pid_t _pid;

    if (stack == 0) {
        perror("malloc err");
        exit(1);
    }

    _pid = clone(start_routine, (char *)stack + STACK_SIZE,
                 SIGCHLD | CLONE_VM | CLONE_FS | CLONE_FILES,
                 arg); // CLONE_SIGHAND 시그널 공유?
    kill(_pid, SIGSTOP);

    int tid = 0;

    while (pThreadTbEnt[tid].bUsed == 1) {
        tid++;
    }

    pThreadTbEnt[tid].bUsed = 1;
    Thread *newNode = stack; // 이게 맞나?
    pThreadTbEnt[tid].pThread = newNode;
    pThreadTbEnt[tid].pThread->pid = _pid;
    pThreadTbEnt[tid].pThread->status = THREAD_STATUS_READY;

    pThreadTbEnt[tid].pThread->stackAddr = stack;

    pThreadTbEnt[tid].pThread->stackSize = STACK_SIZE;
    pThreadTbEnt[tid].pThread->cpu_time = 0;

    *thread = tid;

    if (ReadyQueue.queueCount == 0) { // 아무것도 없을 때

        ReadyQueue.pHead = pThreadTbEnt[tid].pThread;
        ReadyQueue.pTail = pThreadTbEnt[tid].pThread;
        pThreadTbEnt[tid].pThread->pPrev = NULL;
        pThreadTbEnt[tid].pThread->pNext = NULL;
        ReadyQueue.queueCount++;

    } else {

        pThreadTbEnt[tid].pThread->pPrev = ReadyQueue.pTail;
        ReadyQueue.pTail->pNext = pThreadTbEnt[tid].pThread;
        ReadyQueue.pTail = pThreadTbEnt[tid].pThread;
        pThreadTbEnt[tid].pThread->pNext = NULL;
        ReadyQueue.queueCount++;
    }

    return 0;
}

int thread_suspend(
    thread_t tid) { // ready queue에 있는 정보는 삭제해야 하나? ㅇㅇ

    if (pThreadTbEnt[tid].pThread->status == THREAD_STATUS_WAIT) {
        return -1;
    }

    if (pThreadTbEnt[tid].pThread->pPrev != NULL &&
        pThreadTbEnt[tid].pThread->pNext != NULL) {
        if (ReadyQueue.pHead->pid ==
            pThreadTbEnt[tid].pThread->pid) { // 레디큐 헤드값일 때
            ReadyQueue.pHead = pThreadTbEnt[tid].pThread->pNext;
        }
        pThreadTbEnt[tid].pThread->pPrev->pNext =
            pThreadTbEnt[tid].pThread->pNext;

        pThreadTbEnt[tid].pThread->pNext->pPrev =
            pThreadTbEnt[tid].pThread->pPrev;

    } else if (pThreadTbEnt[tid].pThread->pPrev == NULL &&
               pThreadTbEnt[tid].pThread->pNext != NULL) {

        pThreadTbEnt[tid].pThread->pNext->pPrev = NULL;
        ReadyQueue.pHead = pThreadTbEnt[tid].pThread->pNext;

    } else if (pThreadTbEnt[tid].pThread->pPrev != NULL &&
               pThreadTbEnt[tid].pThread->pNext == NULL) {

        pThreadTbEnt[tid].pThread->pPrev->pNext = NULL;
        ReadyQueue.pTail = pThreadTbEnt[tid].pThread->pPrev;
    }
    ReadyQueue.queueCount--;

    if (WaitingQueue.queueCount == 0) { // 아무것도 없을 때

        WaitingQueue.pHead = pThreadTbEnt[tid].pThread;
        WaitingQueue.pTail = pThreadTbEnt[tid].pThread;
        pThreadTbEnt[tid].pThread->pPrev = NULL;
        pThreadTbEnt[tid].pThread->pNext = NULL;
        WaitingQueue.queueCount++;
    } else {

        pThreadTbEnt[tid].pThread->pPrev = ReadyQueue.pTail;
        WaitingQueue.pTail->pNext = pThreadTbEnt[tid].pThread;
        WaitingQueue.pTail = pThreadTbEnt[tid].pThread;
        pThreadTbEnt[tid].pThread->pNext = NULL;
        WaitingQueue.queueCount++;
    }

    pThreadTbEnt[tid].pThread->status = THREAD_STATUS_WAIT;

    // kill(pThreadTbEnt[tid].pThread->pid, SIGSTOP);

    return 0;
}

int thread_cancel(thread_t tid) {
    if (pThreadTbEnt[tid].bUsed == 0) {
        return -1;
    }

    kill(pThreadTbEnt[tid].pThread->pid, SIGKILL);

    if (pThreadTbEnt[tid].pThread->status == THREAD_STATUS_READY ||
        THREAD_STATUS_RUN) {
        ReadyQueue.queueCount--;

        if (pThreadTbEnt[tid].pThread->pPrev != NULL &&
            pThreadTbEnt[tid].pThread->pNext != NULL) {

            if (ReadyQueue.pHead->pid ==
                pThreadTbEnt[tid].pThread->pid) { // 레디큐 헤드값일 때
                ReadyQueue.pHead = pThreadTbEnt[tid].pThread->pNext;
            }
            pThreadTbEnt[tid].pThread->pPrev->pNext =
                pThreadTbEnt[tid].pThread->pNext;

            pThreadTbEnt[tid].pThread->pNext->pPrev =
                pThreadTbEnt[tid].pThread->pPrev;

        } else if (pThreadTbEnt[tid].pThread->pPrev == NULL &&
                   pThreadTbEnt[tid].pThread->pNext != NULL) {
            if (ReadyQueue.pHead->pid ==
                pThreadTbEnt[tid].pThread->pid) { // 레디큐 헤드값일 때
                ReadyQueue.pHead = pThreadTbEnt[tid].pThread->pNext;
            }

            pThreadTbEnt[tid].pThread->pNext->pPrev = NULL;
            ReadyQueue.pHead = pThreadTbEnt[tid].pThread->pNext;

        } else if (pThreadTbEnt[tid].pThread->pPrev != NULL &&
                   pThreadTbEnt[tid].pThread->pNext == NULL) {
            if (ReadyQueue.pHead->pid ==
                pThreadTbEnt[tid].pThread->pid) { // 레디큐 헤드값일 때
                ReadyQueue.pHead = NULL;
                ReadyQueue.pTail = NULL;
            } else {
                pThreadTbEnt[tid].pThread->pPrev->pNext = NULL;
                ReadyQueue.pTail = pThreadTbEnt[tid].pThread->pPrev;
            }
        }
    } else if (pThreadTbEnt[tid].pThread->status == THREAD_STATUS_WAIT) {
        WaitingQueue.queueCount--;

        if (pThreadTbEnt[tid].pThread->pPrev != NULL &&
            pThreadTbEnt[tid].pThread->pNext != NULL) {

            if (WaitingQueue.pHead->pid ==
                pThreadTbEnt[tid].pThread->pid) { // 레디큐 헤드값일 때
                WaitingQueue.pHead = pThreadTbEnt[tid].pThread->pNext;
            }
            pThreadTbEnt[tid].pThread->pPrev->pNext =
                pThreadTbEnt[tid].pThread->pNext;

            pThreadTbEnt[tid].pThread->pNext->pPrev =
                pThreadTbEnt[tid].pThread->pPrev;

        } else if (pThreadTbEnt[tid].pThread->pPrev == NULL &&
                   pThreadTbEnt[tid].pThread->pNext != NULL) {
            if (WaitingQueue.pHead->pid ==
                pThreadTbEnt[tid].pThread->pid) { // 레디큐 헤드값일 때
                WaitingQueue.pHead = pThreadTbEnt[tid].pThread->pNext;
            }

            pThreadTbEnt[tid].pThread->pNext->pPrev = NULL;
            WaitingQueue.pHead = pThreadTbEnt[tid].pThread->pNext;

        } else if (pThreadTbEnt[tid].pThread->pPrev != NULL &&
                   pThreadTbEnt[tid].pThread->pNext == NULL) {
            if (WaitingQueue.pHead->pid ==
                pThreadTbEnt[tid].pThread->pid) { // 레디큐 헤드값일 때
                WaitingQueue.pHead = NULL;
                WaitingQueue.pTail = NULL;
            } else {
                pThreadTbEnt[tid].pThread->pPrev->pNext = NULL;
                WaitingQueue.pTail = pThreadTbEnt[tid].pThread->pPrev;
            }
        }
    }

    pThreadTbEnt[tid].bUsed == 0;
    free(pThreadTbEnt[tid].pThread);
}

int thread_resume(thread_t tid) {
    if (pThreadTbEnt[tid].pThread->status != THREAD_STATUS_WAIT) {
        return -1;
    }

    if (pThreadTbEnt[tid].pThread->pPrev != NULL &&
        pThreadTbEnt[tid].pThread->pNext != NULL) {
        pThreadTbEnt[tid].pThread->pPrev->pNext =
            pThreadTbEnt[tid].pThread->pNext;

        pThreadTbEnt[tid].pThread->pNext->pPrev =
            pThreadTbEnt[tid].pThread->pPrev;

    } else if (pThreadTbEnt[tid].pThread->pPrev == NULL &&
               pThreadTbEnt[tid].pThread->pNext != NULL) {
        pThreadTbEnt[tid].pThread->pNext->pPrev = NULL;
        WaitingQueue.pHead = pThreadTbEnt[tid].pThread->pNext;
    } else if (pThreadTbEnt[tid].pThread->pPrev != NULL &&
               pThreadTbEnt[tid].pThread->pNext == NULL) {
        pThreadTbEnt[tid].pThread->pPrev->pNext = NULL;
        WaitingQueue.pTail = pThreadTbEnt[tid].pThread->pPrev;
    }
    WaitingQueue.queueCount--;

    pThreadTbEnt[tid].pThread->status = THREAD_STATUS_READY;

    if (ReadyQueue.queueCount == 0) { // 아무것도 없을 때

        ReadyQueue.pHead = pThreadTbEnt[tid].pThread;
        ReadyQueue.pTail = pThreadTbEnt[tid].pThread;
        pThreadTbEnt[tid].pThread->pPrev = NULL;
        pThreadTbEnt[tid].pThread->pNext = NULL;
        ReadyQueue.queueCount++;
    } else {

        pThreadTbEnt[tid].pThread->pPrev = ReadyQueue.pTail;
        ReadyQueue.pTail->pNext = pThreadTbEnt[tid].pThread;
        ReadyQueue.pTail = pThreadTbEnt[tid].pThread;
        pThreadTbEnt[tid].pThread->pNext = NULL;
        ReadyQueue.queueCount++;
    }

    return 0;
}

thread_t thread_self(void) {
    thread_t tid = 0;
    // while (pCurrentThread->stackAddr != pThreadTbEnt[tid].pThread->stackAddr)
    // {
    //     tid++;
    // }

    while (1) {
        if (pThreadTbEnt[tid].pThread != NULL) {
            if (pCurrentThread->stackAddr ==
                pThreadTbEnt[tid].pThread->stackAddr) {
                break;
            }
        }
        tid++;
    }

    return tid;
}

void joinHandler(int signum) {
    if (signum == SIGCHLD) {
        return;
    }
}

int thread_join(thread_t tid) { // 해당 스레드가 종료될 때 까지 기다린다.
    // running 하고 있는 쓰레드가 호출한다.
    // waiting status 로 변경, waiting Queue로 이동
    // int realPid = getpid();
    int i = 0;
    pid_t parentTid = 0;
    while (pCurrentThread->stackAddr != pThreadTbEnt[i].pThread->stackAddr) {
        i++;
    }
    parentTid = i;

    thread_t _tid = tid;

    if (WaitingQueue.queueCount < 1) {

        WaitingQueue.pHead = pCurrentThread;
        WaitingQueue.pTail = pCurrentThread;
        pCurrentThread->pPrev = NULL;
        pCurrentThread->pNext = NULL;
        WaitingQueue.queueCount++;

    } else {

        pCurrentThread->pPrev =
            WaitingQueue.pTail; // ReadyQueue  waiting queue 아닌가?
        WaitingQueue.pTail->pNext = pCurrentThread;
        WaitingQueue.pTail = pCurrentThread;
        pCurrentThread->pNext = NULL;
        WaitingQueue.queueCount++;
    }

    if (pCurrentThread->pPrev != NULL && pCurrentThread->pNext != NULL) {
        pThreadTbEnt[parentTid].pThread->pPrev->pNext =
            pThreadTbEnt[parentTid].pThread->pNext;

        pThreadTbEnt[parentTid].pThread->pNext->pPrev =
            pThreadTbEnt[parentTid].pThread->pPrev;
        // ReadyQueue.queueCount--;
    } else {
        pCurrentThread = NULL;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = joinHandler;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {

        return;
    }
    while (pThreadTbEnt[tid].pThread->status != THREAD_STATUS_ZOMBIE) {
        pause();
    }

    if (pThreadTbEnt[parentTid].pThread->pPrev != NULL &&
        pThreadTbEnt[parentTid].pThread->pNext != NULL) {
        pThreadTbEnt[parentTid].pThread->pPrev->pNext =
            pThreadTbEnt[parentTid].pThread->pNext;

        pThreadTbEnt[parentTid].pThread->pNext->pPrev =
            pThreadTbEnt[parentTid].pThread->pPrev;

    } else if (pThreadTbEnt[parentTid].pThread->pPrev == NULL &&
               pThreadTbEnt[parentTid].pThread->pNext != NULL) {
        pThreadTbEnt[parentTid].pThread->pNext->pPrev = NULL;
        WaitingQueue.pHead = pThreadTbEnt[parentTid].pThread->pNext;
    } else if (pThreadTbEnt[parentTid].pThread->pPrev != NULL &&
               pThreadTbEnt[parentTid].pThread->pNext == NULL) {

        pThreadTbEnt[parentTid].pThread->pPrev->pNext = NULL;
        WaitingQueue.pTail = pThreadTbEnt[parentTid].pThread->pPrev;
    }
    WaitingQueue.queueCount--;

    // SIGCHLD 시그널 오면 ready Queue로 이동, status-> ready
    if (ReadyQueue.queueCount < 1) { // (2)
        pThreadTbEnt[parentTid].pThread->status = THREAD_STATUS_READY;

        ReadyQueue.pHead = pThreadTbEnt[parentTid].pThread;
        ReadyQueue.pTail = pThreadTbEnt[parentTid].pThread;
        pThreadTbEnt[parentTid].pThread->pPrev = NULL;
        pThreadTbEnt[parentTid].pThread->pNext = NULL;
        ReadyQueue.queueCount++;

    } else {
        pThreadTbEnt[parentTid].pThread->status = THREAD_STATUS_READY;

        pThreadTbEnt[parentTid].pThread->pPrev = ReadyQueue.pTail;

        if (ReadyQueue.pTail == NULL) {
            ReadyQueue.pTail = pThreadTbEnt[parentTid].pThread;
        } else {
            ReadyQueue.pTail->pNext = pThreadTbEnt[parentTid].pThread;
            ReadyQueue.pTail = pThreadTbEnt[parentTid].pThread;
        }
        pThreadTbEnt[parentTid].pThread->pNext = NULL;
        ReadyQueue.queueCount++;
    }

    kill(pThreadTbEnt[parentTid].pThread->pid, SIGSTOP);

    pThreadTbEnt[tid].bUsed == 0;
    free(pThreadTbEnt[_tid].pThread);
    pThreadTbEnt[tid].pThread = NULL;
    return 0;
}

int thread_cputime(void) { return pCurrentThread->cpu_time; }

void thread_exit(void) { pCurrentThread->status = THREAD_STATUS_ZOMBIE; }
