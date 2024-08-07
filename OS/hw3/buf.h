#ifndef __HW2_H__
#define __HW2_H__

#include "queue.h"


#define MAX_BUFLIST_NUM     (4)
#define MAX_BUF_NUM         (16)   
#define MAX_BUF_STATE_NUM   (2)
#define BLKNO_INVALID       (-1)

typedef struct Buf Buf;

typedef enum {
    FALSE = 0,
    TRUE = 1
} BOOL;


typedef enum __BufState
{
    BUF_STATE_CLEAN,
    BUF_STATE_DIRTY
} BufState;

typedef enum __BufStateList
{
    BUF_CLEAN_LIST = 0,
    BUF_DIRTY_LIST = 1
} BufStateList;

struct Buf
{
    int          blkno;
    BufState     state;
    void*        pMem;
    CIRCLEQ_ENTRY(Buf) blist;
    CIRCLEQ_ENTRY(Buf) slist;
    CIRCLEQ_ENTRY(Buf) llist;
    CIRCLEQ_ENTRY(Buf) flist;   // link for free list
};

extern CIRCLEQ_HEAD(bufList, Buf) bufList[MAX_BUFLIST_NUM];
extern CIRCLEQ_HEAD(stateList, Buf) stateList[MAX_BUF_STATE_NUM];
extern CIRCLEQ_HEAD(freeList, Buf) freeListHead;
extern CIRCLEQ_HEAD(lruList, Buf) lruListHead;


extern void BufInit(void);
extern void BufRead(int blkno, char* pData);
extern void BufWrite(int blkno, char* pData);
extern void BufSync(void);
extern Buf* BufFind(int blkno);
extern void BufSyncBlock(int blkno);
extern int GetBufInfoInStateList(BufStateList listnum, Buf* ppBufInfo[], int numBuf);
extern int GetBufInfoInLruList(Buf* ppBufInfo[], int numBuf);
extern int GetBufInfoInBufferList(int index, Buf* ppBufInfo[], int numBuf);


#endif

