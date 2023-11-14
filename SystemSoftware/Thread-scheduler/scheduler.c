

#include "scheduler.h"
#include "init.h"
#include "thread.h"

void __ContextSwitch(pid_t curpid, pid_t newpid) {
    pid_t newpid2 = newpid;

    if (curpid == 0) { // 처음 시작할 때

    } else if (newpid == 0) { // 다음 스레드가 없을 때, 자기 자신 반복
        // kill(curpid, SIGSTOP);
        newpid2 = curpid;
        goto NOTHREAD;

    } else if (ReadyQueue.queueCount == 0) { // (2)
        // kill(curpid, SIGSTOP);
        if (pCurrentThread->status == THREAD_STATUS_ZOMBIE) {
            kill(curpid, SIGKILL);
            pCurrentThread = NULL; // add

        } else {
            // kill(curpid, SIGSTOP);
            pCurrentThread->status = THREAD_STATUS_READY;
            ReadyQueue.queueCount++;
            ReadyQueue.pHead = pCurrentThread;
            ReadyQueue.pTail = pCurrentThread;
            pCurrentThread->pPrev = NULL;
            pCurrentThread->pNext = NULL;
        }

    } else {
        // kill(curpid, SIGSTOP);
        if (pCurrentThread->status == THREAD_STATUS_ZOMBIE) {
            kill(curpid, SIGKILL);
            pCurrentThread = NULL;

        } else {

            // kill(curpid, SIGSTOP);
            pCurrentThread->status = THREAD_STATUS_READY;
            ReadyQueue.queueCount++;
            pCurrentThread->pPrev = ReadyQueue.pTail;
            ReadyQueue.pTail->pNext = pCurrentThread;
            ReadyQueue.pTail = pCurrentThread;
            pCurrentThread->pNext = NULL;
        }
    }

    pCurrentThread = ReadyQueue.pHead;
    ReadyQueue.queueCount--;

    if (ReadyQueue.queueCount != 0) {
        ReadyQueue.pHead = ReadyQueue.pHead->pNext;
    }

NOTHREAD:
    if (pCurrentThread->status == THREAD_STATUS_ZOMBIE) {

    } else {
        pCurrentThread->status = THREAD_STATUS_RUN;
    }
    pCurrentThread->cpu_time += TIMESLICE;

    pCurrentThread->pid = newpid2;

    kill(newpid2, SIGCONT);
}

void timer(int sig) {
    if (sig != SIGALRM)
        return;
    if (pCurrentThread == NULL || pCurrentThread->pid == 0) {

    } else {
        kill(pCurrentThread->pid, SIGSTOP);
    }

    if (pCurrentThread == NULL) { // 초기 시작

        pid_t newpid = ReadyQueue.pHead->pid;
        __ContextSwitch(0, newpid);

    } else if (ReadyQueue.queueCount > 0) {

        pid_t curpid = pCurrentThread->pid;
        pid_t newpid = ReadyQueue.pHead->pid;
        if (pCurrentThread->status == THREAD_STATUS_WAIT) {
            curpid = 0;
        }
        Thread *iter;
        int i = 0;
        // for (iter = ReadyQueue.pHead; iter != NULL; iter = iter->pNext) {
        //     printf("%d->", iter->pid);
        //     i++;
        //     if (i == 10)
        //         break;
        // }
        // printf("\ncur %d next %d\n", curpid, newpid);
        __ContextSwitch(curpid, newpid);
    } else if (ReadyQueue.queueCount == 0) {

        pid_t curpid = pCurrentThread->pid;
        // pid_t newpid = ReadyQueue.pHead->pid;

        __ContextSwitch(curpid, 0);
    }

    return;
}

void RunScheduler(void) {

    timer_t timerid;
    struct sigaction sa;
    struct itimerspec its;

    sa.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;
    sa.sa_sigaction = timer;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        printf("err1");
        return;
    }

    if (timer_create(CLOCK_REALTIME, NULL, &timerid) == -1) {
        printf("err2");
        return;
    }

    its.it_interval.tv_sec = TIMESLICE;
    its.it_interval.tv_nsec = 0;

    its.it_value.tv_sec = TIMESLICE; // 초기값은 0으로 하는게 좋을까?
    its.it_value.tv_nsec = 0;

    timer_settime(timerid, 0, &its, NULL);

    return;
}
