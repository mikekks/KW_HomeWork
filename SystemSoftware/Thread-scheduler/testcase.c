#include "init.h"
#include "scheduler.h"
#include "semaphore.h"
#include "thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SEM1_NAME "mysem1"
#define SEM2_NAME "mysem2"

int semid_binary = 0;
int semid_counting = 0;

void *Tc1ThreadProc(void *param) {
    int i = 0;
    int tid;

    tid = thread_self();
    for (i = 0; i < 5; i++) {
        thread_sem_wait(semid_binary);
        sleep(1);
        printf("Tc1ThreadProc: my thread id (%d) has mutex, count is (%d)\n",
               tid, i);
        thread_sem_post(semid_binary);
    }
    thread_exit();
}

void TestCase1(void) {
    int tid[4];
    int i = 0, i1 = 1, i2 = 2, i3 = 3, i4 = 4;

    semid_binary = thread_sem_open(SEM1_NAME, 1);

    thread_create(&tid[0], NULL, Tc1ThreadProc, &i1);
    thread_create(&tid[1], NULL, Tc1ThreadProc, &i2);
    thread_create(&tid[2], NULL, Tc1ThreadProc, &i3);
    thread_create(&tid[3], NULL, Tc1ThreadProc, &i4);

    for (i = 0; i < 4; i++) {
        thread_join(tid[i]);
        printf("Thread [ %d ] is finish\n", tid[i]);
    }

    thread_sem_close(semid_binary);

    printf("\n");
    printf("testcase1 has been fully executed\n");

    return;
}

void *Tc2ThreadProc(void *param) {
    int i = 0;
    int count = 0;
    int tid = 0;

    count = *((int *)param);
    tid = thread_self();
    thread_sem_post(semid_binary);

    for (i = 0; i < count; i++) {
        thread_sem_wait(semid_counting);
        printf(
            "Tc2ThreadProc: my thread id (%d) has a dinner.., count is (%d)\n",
            tid, i);
        sleep(1);
        thread_sem_post(semid_counting);
    }
    thread_exit();
}

void TestCase2(void) {
    int tid[6];
    int i = 0;
    int count1, count2, count3, count4, count5, count6;

    semid_binary = thread_sem_open(SEM1_NAME, 1);
    semid_counting = thread_sem_open(SEM2_NAME, 3);

    thread_sem_wait(semid_binary);
    count1 = 4;
    thread_create(&tid[0], NULL, Tc2ThreadProc, &count1);

    thread_sem_wait(semid_binary);
    count2 = 7;
    thread_create(&tid[1], NULL, Tc2ThreadProc, &count2);

    thread_sem_wait(semid_binary);
    count3 = 3;
    thread_create(&tid[2], NULL, Tc2ThreadProc, &count3);

    thread_sem_wait(semid_binary);
    count4 = 4;
    thread_create(&tid[3], NULL, Tc2ThreadProc, &count4);

    thread_sem_wait(semid_binary);
    count5 = 4;
    thread_create(&tid[4], NULL, Tc2ThreadProc, &count5);

    thread_sem_wait(semid_binary);
    count6 = 5;
    thread_create(&tid[5], NULL, Tc2ThreadProc, &count6);

    for (i = 0; i < 6; i++) {
        thread_join(tid[i]);
        printf("Thread [ %d ] is finish\n", tid[i]);
    }

    thread_sem_close(semid_binary);
    thread_sem_close(semid_counting);

    return;
}

int main(int argc, char *argv[]) {

    int TcNum;
    thread_t tid1, tid2;

    if (argc != 2) {
        printf("Input TestCase Number!\n");
        exit(0);
    }

    Init();

    TcNum = atoi(argv[1]);

    switch (TcNum) {
    case 1:
        thread_create(&tid1, NULL, (void *)TestCase1, 0);
        break;
    case 2:
        thread_create(&tid2, NULL, (void *)TestCase2, 0);
        break;
    }

    RunScheduler();

    while (1) {
    }

    return 0;
}
