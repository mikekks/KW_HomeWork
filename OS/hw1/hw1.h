#ifndef __HW1_H__
#define __HW1_H__

#include "queue.h"


#define HASH_TBL_SIZE (8)
#define MAX_LIST_NUM  (3)


typedef struct Object Object;

typedef enum {
    FALSE = 0,
    TRUE = 1
} BOOL;


typedef enum __List
{
    LIST1 = 0,
    LIST2 = 1,
    LIST3 = 2
} List;

struct Object
{
    int                     objnum;
    CIRCLEQ_ENTRY(Object)   hash;
    CIRCLEQ_ENTRY(Object)   link;
};


extern CIRCLEQ_HEAD(hashTable, Object) ppHashTable[HASH_TBL_SIZE];
extern CIRCLEQ_HEAD(list, Object) ppListHead[MAX_LIST_NUM];



extern void     Init(void);
extern void     InsertObjectToTail(Object* pObj, int objNum, List listNum);
extern void     InsertObjectToHead(Object* pObj, int objNum, List listNum);
extern Object*  FindObjectByNum(int objnum);
extern BOOL     DeleteObject(Object* pObj);
extern BOOL     DeleteObjectByNum(int objnum);
extern int      EnumberateObjectsByListNum(List listnum, Object* ppObject[], int count);
extern int      EnumberateObjectsByHashIndex(int index, Object* ppObject[], int count);




#endif /* __HW1_H__ */