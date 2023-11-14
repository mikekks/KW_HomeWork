#include "semaphore.h"
#include "thread.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

SemaphoreTblEnt pSemaphoreTblEnt[MAX_SEMAPHORE_NUM];

int thread_sem_open(char *name, int count) {
    int pos = 0;

    while (pSemaphoreTblEnt[pos].bUsed != 0) {
        pos++;
    }

    strcpy(pSemaphoreTblEnt[pos].name, name);
    pSemaphoreTblEnt[pos].bUsed = 1;

    pSemaphoreTblEnt[pos].pSemaphore =
        malloc(sizeof(int) + sizeof(ThreadQueue));
    pSemaphoreTblEnt[pos].pSemaphore->count = count;

    return pos;
}

int thread_sem_wait(int semid) {

    int tid = thread_self();

START:
    tid = thread_self();
    // printf("WAIT Come tid %d\n", tid);
    if (pSemaphoreTblEnt[semid].pSemaphore->count ==
        0) { // waiting queue 에 넣기
             // printf("!!!!!count 0\n");
        if (pSemaphoreTblEnt[semid].pSemaphore->waitingQueue.queueCount ==
            0) { // 아무것도 없을 때

            pSemaphoreTblEnt[semid].pSemaphore->waitingQueue.pHead =
                pThreadTbEnt[tid].pThread;
            pSemaphoreTblEnt[semid].pSemaphore->waitingQueue.pTail =
                pThreadTbEnt[tid].pThread;
            pThreadTbEnt[tid].pThread->pPrev = NULL;
            pThreadTbEnt[tid].pThread->pNext = NULL;
            pSemaphoreTblEnt[semid].pSemaphore->waitingQueue.queueCount++;
        } else {

            pThreadTbEnt[tid].pThread->pPrev = ReadyQueue.pTail;
            pSemaphoreTblEnt[semid].pSemaphore->waitingQueue.pTail->pNext =
                pThreadTbEnt[tid].pThread;
            pSemaphoreTblEnt[semid].pSemaphore->waitingQueue.pTail =
                pThreadTbEnt[tid].pThread;
            pThreadTbEnt[tid].pThread->pNext = NULL;
            pSemaphoreTblEnt[semid].pSemaphore->waitingQueue.queueCount++;
        }

        pThreadTbEnt[tid].pThread->status = THREAD_STATUS_WAIT;

        kill(pThreadTbEnt[tid].pThread->pid, SIGSTOP);

        if (pSemaphoreTblEnt[semid].pSemaphore->count == 0)
            goto START;
        else
            pSemaphoreTblEnt[semid].pSemaphore->count--;

    } else {
        pSemaphoreTblEnt[semid].pSemaphore->count--;
        // printf("COUNT %d\n", pSemaphoreTblEnt[semid].pSemaphore->count);
    }

    return 0;
}

int thread_sem_post(int semid) {

    if (pSemaphoreTblEnt[semid].pSemaphore->waitingQueue.queueCount == 0) {
        pSemaphoreTblEnt[semid].pSemaphore->count++;

        return 0;
    }

    pSemaphoreTblEnt[semid].pSemaphore->count++;
    // printf("AFTER POST COUNT %d\n",
    // pSemaphoreTblEnt[semid].pSemaphore->count);
    int tid = 0;
    while (1) {
        if (pThreadTbEnt[tid].pThread != NULL) {
            if (pThreadTbEnt[tid].pThread->pid ==
                pSemaphoreTblEnt[semid].pSemaphore->waitingQueue.pHead->pid) {
                break;
            }
        }
        tid++;
    }

    // waiting queue에서 빼서
    if (pThreadTbEnt[tid].pThread->pPrev != NULL &&
        pThreadTbEnt[tid].pThread->pNext != NULL) {
        pThreadTbEnt[tid].pThread->pPrev->pNext =
            pThreadTbEnt[tid].pThread->pNext;

        pThreadTbEnt[tid].pThread->pNext->pPrev =
            pThreadTbEnt[tid].pThread->pPrev;

    } else if (pThreadTbEnt[tid].pThread->pPrev == NULL &&
               pThreadTbEnt[tid].pThread->pNext != NULL) {
        pThreadTbEnt[tid].pThread->pNext->pPrev = NULL;
        pSemaphoreTblEnt[semid].pSemaphore->waitingQueue.pHead =
            pThreadTbEnt[tid].pThread->pNext;
    } else if (pThreadTbEnt[tid].pThread->pPrev != NULL &&
               pThreadTbEnt[tid].pThread->pNext == NULL) {
        pThreadTbEnt[tid].pThread->pPrev->pNext = NULL;
        pSemaphoreTblEnt[semid].pSemaphore->waitingQueue.pTail =
            pThreadTbEnt[tid].pThread->pPrev;
    }
    pSemaphoreTblEnt[semid].pSemaphore->waitingQueue.queueCount--;
    // pSemaphoreTblEnt[semid].pSemaphore->count--;

    // readyqueue에 넣기
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

int thread_sem_close(int semid) {

    pSemaphoreTblEnt[semid].bUsed = 0;
    memset(pSemaphoreTblEnt[semid].name, 0, SEM_NAME_MAX);
    free(pSemaphoreTblEnt[semid].pSemaphore);
    return 0;
}
