#include "buf.h"
#include "disk.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct bufList bufList[MAX_BUFLIST_NUM];
struct stateList stateList[MAX_BUF_STATE_NUM];
struct freeList freeListHead;
struct lruList lruListHead;

Buf* findBufInLruList(int blkno);
BOOL findBufInStateList(BufStateList listnum, Buf *buf);
void BufInsertToHead(Buf *pBuf, int blkno, BufStateList listNum);
void BufInsertToTail(Buf *pBuf, int blkno, BufStateList listNum);
Buf* BufGetNewBuffer();
BOOL BufDelete(Buf *pBuf);
void GetBufInfoByHashIndex(int index, Buf **ppBufInfo, int *pNumBuf);
int GetBufCount();
void processLruListHead();


void BufInit(void) {
    for (int i = 0; i < MAX_BUFLIST_NUM; i++) {
        CIRCLEQ_INIT(&bufList[i]);
    }

    for (int i = 0; i < MAX_BUF_STATE_NUM; i++) {
        CIRCLEQ_INIT(&stateList[i]);
    }

    CIRCLEQ_INIT(&freeListHead);
    CIRCLEQ_INIT(&lruListHead);

    // buffer cache 초기화
    for (int i = 0; i < MAX_BUF_NUM; i++) {
        Buf *buf = malloc(sizeof(Buf));
        buf->pMem = malloc(sizeof(BLOCK_SIZE));
        buf->blkno = i;
        CIRCLEQ_INSERT_TAIL(&freeListHead, buf, flist);
    }
}

Buf *BufFind(int blkno) {

    int bufNum = blkno % MAX_BUFLIST_NUM;
    Buf *iter = CIRCLEQ_FIRST(&bufList[bufNum]);

    while (iter != (void *)&bufList[bufNum]) {
        if (iter->blkno == blkno) {
            return iter;
        }

        iter = CIRCLEQ_NEXT(iter, blist);
    }

    return NULL;
}

void BufRead(int blkno, char *pData) {
    // bufferCache(buffer list)에서 확인
    // 있을 경우
    // pData에 write

    // 없을 경우
    // 최대 버퍼 갯수(MAX_BUF_NUM) 초과했을 경우, 재사용 고려
    // LRU list head가 clean list일 경우
    // LRU list 에 삭제
    // buffer list에서 삭제
    // clean list에서 삭제

    // LRU list head가 dirty list일 경우
    // LRU list 에 삭제
    // buffer list에서 삭제
    // dirty list에서 삭제
    // DevWriteBlock() 호출을 통해 디스크에 저장

    // Buf 할당 + blk no 세팅
    // pMem 메모리 할당
    // disk에서 DevReadBlock() 불러오기
    // buffer list의 head에 넣기
    // clean list의 tail에 넣기
    // pMem에 데이터를 pData에 깊은 복사(write)

    // 이미 LRU list에 있을 경우
    // LRU list에서 삭제
    // LRU list Tail로 이동

    Buf *buf = BufFind(blkno);
    // 깊은 복사 해야하나??

    if (buf != NULL) {
        memcpy(pData, buf->pMem, BLOCK_SIZE);
    } else {
        int count = GetBufCount();
        if (count <= 0) {
            processLruListHead();
        }

        buf = BufGetNewBuffer();
        buf->blkno = blkno;
        buf->pMem = malloc(BLOCK_SIZE);
        buf->state = BUF_STATE_CLEAN;
        DevReadBlock(blkno, buf->pMem);
        BufInsertToHead(buf, blkno, BUF_CLEAN_LIST);
        memcpy(pData, buf->pMem, BLOCK_SIZE);
    }

    Buf *bufInLruList = findBufInLruList(blkno);
    if (bufInLruList != NULL) {
        CIRCLEQ_REMOVE(&lruListHead, buf, llist);
    }

    CIRCLEQ_INSERT_TAIL(&lruListHead, buf, llist);
}

void BufWrite(int blkno, char *pData) {

    // bufferCache(buffer list)에서 확인
        // 있을 경우
            // dirty list에 있었으면
                // 해당 블럭의 pMem에 pData를 복사(write)
            // clean list에 있었으면
                // 해당 블럭의 pMem에 pData를 복사(write)
                // clean list에서 삭제하고, dirty list의 tail로 이동

        // 없을 경우
            // 최대 버퍼 갯수(MAX_BUF_NUM) 초과했을 경우, 재사용 고려
                // LRU list head가 clean list일 경우
                // LRU list 에 삭제
                // buffer list에서 삭제
                // clean list에서 삭제

                // LRU list head가 dirty list일 경우
                // LRU list 에 삭제
                // buffer list에서 삭제
                // dirty list에서 삭제
                // DevWriteBlock() 호출을 통해 디스크에 저장

    // Buf 할당 + blk no 세팅
    // pMem 메모리 할당
    // pData의 데이터를 pMem에 복사
    // buffer list의 head에 넣기
    // dirty list의 tail에 넣기

    // 이미 LRU list에 있을 경우
    // LRU list에서 삭제
    // LRU list Tail로 이동

    Buf *buf = BufFind(blkno);
    // 깊은 복사 해야하나??
    //printf("TEST buf %d\n", blkno);
    if (buf != NULL) {
        if (findBufInStateList(BUF_CLEAN_LIST, buf) == TRUE) {
            //printf("Not null TEST buf %d\n", blkno);
            memcpy(buf->pMem, pData, BLOCK_SIZE);
            CIRCLEQ_REMOVE(&stateList[BUF_CLEAN_LIST], buf, slist);
            CIRCLEQ_INSERT_TAIL(&stateList[BUF_DIRTY_LIST], buf, slist);
            buf->state = BUF_STATE_DIRTY;

        } else if (findBufInStateList(BUF_DIRTY_LIST, buf) == TRUE) {
            // [이건 아닌듯????] 먼저 해당 블럭 Sync
            // 해당 블럭의 pMem에 pData를 복사(write)
            memcpy(buf->pMem, pData, BLOCK_SIZE);
        }
    } else {
        int count = GetBufCount();
        if (count <= 0) {
            processLruListHead();
        }

        buf = BufGetNewBuffer();
        buf->blkno = blkno;
        buf->pMem = malloc(BLOCK_SIZE);
        buf->state = BUF_STATE_DIRTY;
        BufInsertToHead(buf, blkno, BUF_DIRTY_LIST);
        memcpy(buf->pMem, pData, BLOCK_SIZE);
    }

    Buf *bufInLruList = findBufInLruList(blkno);
    if (bufInLruList != NULL) {
        CIRCLEQ_REMOVE(&lruListHead, buf, llist);
    }

    CIRCLEQ_INSERT_TAIL(&lruListHead, buf, llist);
}

void BufSync(void) {
    // dirty list head부터 block 단위로 동작(for문)
    // DevWriteBlock() 호출을 통해 디스크에 저장
    // clean list tail로 이동, , clean state 설정

    Buf *iter = CIRCLEQ_FIRST(&stateList[BUF_DIRTY_LIST]);

    while (iter != (void *)&stateList[BUF_DIRTY_LIST]) {
        Buf* next = CIRCLEQ_NEXT(iter, slist);

        DevWriteBlock(iter->blkno, iter->pMem);
        CIRCLEQ_REMOVE(&stateList[BUF_DIRTY_LIST], iter, slist);
        CIRCLEQ_INSERT_TAIL(&stateList[BUF_CLEAN_LIST], iter, slist);
        iter->state = BUF_STATE_CLEAN;

        iter = next;
    }
}

void BufSyncBlock(int blkno) {
    // clean list에 있는 경우 아무 동작 X
    // dirty list에 있는 경우
    // dirty list에 꺼내기
    // DevWriteBlock() 호출을 통해 디스크에 저장
    // clean list로 이동, clean state 설정

    Buf *buf = BufFind(blkno);

    if (buf->state == BUF_STATE_DIRTY) {
        CIRCLEQ_REMOVE(&stateList[BUF_DIRTY_LIST], buf, slist);
        DevWriteBlock(buf->blkno, buf->pMem);
        CIRCLEQ_INSERT_TAIL(&stateList[BUF_CLEAN_LIST], buf, slist);
        buf->state = BUF_STATE_CLEAN;
    }
}

int GetBufInfoInStateList(BufStateList listnum, Buf *ppBufInfo[], int numBuf) {
    // numBuf 용도가 뭐지?
    Buf *retBufs[numBuf];

    Buf *iter = CIRCLEQ_FIRST(&stateList[listnum]);
    int cnt = 0;

    while (iter != (void *)&stateList[listnum]) {
        retBufs[cnt++] = iter;
        iter = CIRCLEQ_NEXT(iter, slist);
    }

    memcpy(ppBufInfo, retBufs, sizeof(Buf*) * numBuf); // 사이즈 이게 맞는지 검증 필요

    return cnt;
}

int GetBufInfoInLruList(Buf *ppBufInfo[], int numBuf) {
    Buf *retBufs[numBuf];

    Buf *iter = CIRCLEQ_FIRST(&lruListHead);
    int cnt = 0;

    while (iter != (void *)&lruListHead) {
        retBufs[cnt++] = iter;
        iter = CIRCLEQ_NEXT(iter, llist);
    }

    memcpy(ppBufInfo, retBufs, sizeof(Buf*) * numBuf); // 사이즈 이게 맞는지 검증 필요

    return cnt;
 }

int GetBufInfoInBufferList(int index, Buf *ppBufInfo[], int numBuf) {
    Buf *retBufs[numBuf];

    Buf *iter = CIRCLEQ_FIRST(&bufList[index]);
    int cnt = 0;

    while (iter != (void *)&bufList[index]) {
        retBufs[cnt++] = iter;
        iter = CIRCLEQ_NEXT(iter, blist);
    }

    memcpy(ppBufInfo, retBufs, sizeof(Buf*) * numBuf); // 사이즈 이게 맞는지 검증 필요

    return cnt;
}

/////// 내부 함수들

Buf* findBufInLruList(int blkno) {
    Buf *iter = CIRCLEQ_FIRST(&lruListHead);
    while (iter != (void *)&lruListHead) {
        if (iter->blkno == blkno) {
            return iter;
        }

        iter = CIRCLEQ_NEXT(iter, llist);
    }

    return NULL;
}

BOOL findBufInStateList(BufStateList listnum, Buf *buf) {
    Buf *iter = CIRCLEQ_FIRST(&stateList[listnum]);
    while (iter != (void *)&stateList[listnum]) {
        if (iter->blkno == buf->blkno) {
            return TRUE;
        }

        iter = CIRCLEQ_NEXT(iter, slist);
    }

    return FALSE;
}

void BufInsertToHead(Buf *pBuf, int blkno, BufStateList listNum) {
    pBuf->blkno = blkno;
    int idx = blkno % MAX_BUFLIST_NUM;
    CIRCLEQ_INSERT_HEAD(&bufList[idx], pBuf, blist);
    CIRCLEQ_INSERT_TAIL(&stateList[listNum], pBuf, slist);
}

//void BufInsertToTail(Buf *pBuf, int blkno, BufStateList listNum) {}

Buf* BufGetNewBuffer() {
    // free list의 head의 Buf 리턴
    Buf *buf = CIRCLEQ_FIRST(&freeListHead);
    CIRCLEQ_REMOVE(&freeListHead, buf, flist);

    return buf;
}

BOOL BufDelete(Buf *pBuf) {
    // buffer list에서 삭제
    // free list로 이동 - 검증 필요!!!!!

    int idx = pBuf->blkno % MAX_BUFLIST_NUM;
    Buf *iter = CIRCLEQ_FIRST(&bufList[idx]);

    while (iter != (void *)&bufList[idx]) {
        if (iter->blkno == pBuf->blkno) {
            CIRCLEQ_REMOVE(&bufList[idx], iter, blist);
            CIRCLEQ_INSERT_TAIL(&freeListHead, iter, flist);

            return TRUE;
        }

        iter = CIRCLEQ_NEXT(iter, blist);
    }

    return FALSE;
}

// void GetBufInfoByHashIndex(int index, Buf **ppBufInfo, int *pNumBuf) {
//     // EnumObjectByHashIndex 참고
// }

int GetBufCount() {
    Buf *iter = CIRCLEQ_FIRST(&freeListHead);
    int cnt = 0;

    while (iter != (void *)&freeListHead) {
        cnt++;
        iter = CIRCLEQ_NEXT(iter, flist);
    }

    return cnt;
}

void processLruListHead() {
    Buf *iter = CIRCLEQ_FIRST(&lruListHead);
    CIRCLEQ_REMOVE(&lruListHead, iter, llist);
    BufDelete(iter);

    if (iter->state == BUF_STATE_CLEAN) {
        CIRCLEQ_REMOVE(&stateList[BUF_CLEAN_LIST], iter, slist);
    } else {
        CIRCLEQ_REMOVE(&stateList[BUF_DIRTY_LIST], iter, slist);
        DevWriteBlock(iter->blkno, iter->pMem);
    }
}
