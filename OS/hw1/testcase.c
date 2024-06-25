#include <stdio.h>
#include <stdlib.h>
#include "hw1.h"

#define NUM_OBJECTS  96
#define PRIME_NUMBER_ARRAY_SIZE 5

int PRIME_NUMBER_ARRAY[PRIME_NUMBER_ARRAY_SIZE] = {2, 3, 5, 7, 11};

void printList() {
    int i, j;
    int count = 0;
    Object* ppObject[NUM_OBJECTS];

    for (i = 0; i < MAX_LIST_NUM; ++i) {
        count = EnumberateObjectsByListNum(i, ppObject, NUM_OBJECTS);
    
        printf("List %d:[", i + 1);
        for (j = 0; j < count; ++j) printf(" %2d", ppObject[j]->objnum);
        printf("]\n");
    }
}

void printHashTable() {
    int i, j;
    int count = 0;
    Object* ppObject[NUM_OBJECTS];

    for (i = 0; i < HASH_TBL_SIZE; ++i) {
        count = EnumberateObjectsByHashIndex(i, ppObject, NUM_OBJECTS);
    
        printf("HashTable %d[:", i + 1);
        for (j = 0; j < count; ++j) printf(" %2d", ppObject[j]->objnum);
        printf("]\n");
    }
}

void testcase1() {
    int i, j;
    Object* pObj;

    for (i = 0; i < NUM_OBJECTS / 2; ++i) {
        pObj = (Object*)malloc(sizeof(Object));

        ((i / HASH_TBL_SIZE) + (i % HASH_TBL_SIZE)) % 2
        ? InsertObjectToTail(pObj, i, i % MAX_LIST_NUM)
        : InsertObjectToHead(pObj, i, i % MAX_LIST_NUM);
    }

    for (i = NUM_OBJECTS - 1; i >= NUM_OBJECTS / 2; --i) {
        pObj = (Object*)malloc(sizeof(Object));

        ((i / HASH_TBL_SIZE) + (i % HASH_TBL_SIZE)) % 2
        ? InsertObjectToTail(pObj, i, i % MAX_LIST_NUM)
        : InsertObjectToHead(pObj, i, i % MAX_LIST_NUM);
    }

    printList();
    printHashTable();
}

void testcase2() {
    int i, j;
    Object* pObj;
    Object* pFindObj;
    BOOL isInsertedNum[NUM_OBJECTS] = {FALSE, };

    for (i = 0; i < PRIME_NUMBER_ARRAY_SIZE; ++i) {
        for (j = PRIME_NUMBER_ARRAY[i]; j < NUM_OBJECTS; j += PRIME_NUMBER_ARRAY[i]) {
            pFindObj = FindObjectByNum(j);

            if (pFindObj == NULL && !isInsertedNum[j]) {
                pObj = (Object*)malloc(sizeof(Object));

                ((j / HASH_TBL_SIZE) + (j % HASH_TBL_SIZE)) % 2
                ? InsertObjectToTail(pObj, j, j % MAX_LIST_NUM)
                : InsertObjectToHead(pObj, j, j % MAX_LIST_NUM);

                isInsertedNum[j] = TRUE;
            }
            else if (pFindObj != NULL && isInsertedNum[j]) continue;
            else {
                printf("TestCase 2: Fail\n");
                return;
            }
        }
    }
    
    printList();
    printHashTable();
}

void testcase3() {
    int i, j;
    BOOL isDeleted;
    Object* pObj;
    BOOL isInsertedNum[NUM_OBJECTS] = {FALSE, };

    for (i = PRIME_NUMBER_ARRAY_SIZE - 1; i >= 0; --i) {
        for (j = PRIME_NUMBER_ARRAY[i]; j < NUM_OBJECTS; j += PRIME_NUMBER_ARRAY[i]) {
            isDeleted = DeleteObjectByNum(j);

            if (!isDeleted && !isInsertedNum[j]) {
                pObj = (Object*)malloc(sizeof(Object));

                ((j / HASH_TBL_SIZE) + (j % HASH_TBL_SIZE)) % 2
                ? InsertObjectToTail(pObj, j, j % MAX_LIST_NUM)
                : InsertObjectToHead(pObj, j, j % MAX_LIST_NUM);

                isInsertedNum[j] = TRUE;
            }
            else if (isDeleted && isInsertedNum[j]) {
                isInsertedNum[j] = FALSE;
            }
            else {
                printf("TestCase 3: Fail\n");
                return;
            }
        }
    }

    printList();
    printHashTable();

    for (i = 0; i < NUM_OBJECTS; ++i) {
        isDeleted = DeleteObjectByNum(i);
        if (isDeleted != isInsertedNum[i]) {
            printf("TestCase 3: Fail\n");
            return;
        }
    }

    printList();
    printHashTable();
}

void testcase4() {
    int i, j, k;
    Object* pObj;
    Object* pFindObj;
    BOOL isInsertedNum[NUM_OBJECTS] = {FALSE, };

    for (i = 0; i < NUM_OBJECTS; ++i) {
        pObj = (Object*)malloc(sizeof(Object));

        ((i / HASH_TBL_SIZE) + (i % HASH_TBL_SIZE)) % 2
        ? InsertObjectToTail(pObj, i, i % MAX_LIST_NUM)
        : InsertObjectToHead(pObj, i, i % MAX_LIST_NUM);

        isInsertedNum[i] = TRUE;
    }

    for (i = NUM_OBJECTS - 1; i >= 0; --i) {
        for (j = 0; j < PRIME_NUMBER_ARRAY_SIZE; ++j) {
            for(k = i; k > 0; k /= PRIME_NUMBER_ARRAY[j]) {
                pFindObj = FindObjectByNum(k);
                if (pFindObj != NULL) {
                    if (j % 2) {
                        if(DeleteObject(pFindObj) != DeleteObjectByNum(pFindObj->objnum)) {
                            isInsertedNum[i] = FALSE;
                        } else {
                            printf("TestCase 3: Fail\n");
                            return;
                        }
                    } else {
                        if(DeleteObjectByNum(pFindObj->objnum) != DeleteObject(pFindObj)) {
                            isInsertedNum[i] = FALSE;
                        } else {
                            printf("TestCase 3: Fail\n");
                            return;
                        }
                    }
                }
            }
        }
    }

    printList();
    printHashTable();
}

int main(int argc, char* argv[])
{
    int tcNum;

    if(argc!= 2)
    {
        printf("Input TestCase Number!\n");
        return 0;
    }

    tcNum = atoi(argv[1]);
    Init();
    switch(tcNum)
    {
        case 1:
            testcase1();
            break;
        case 2:
            testcase2();
            break;
        case 3:
            testcase3();
            break;
        case 4:
            testcase4();
            break;
    }

    return 0;
}

