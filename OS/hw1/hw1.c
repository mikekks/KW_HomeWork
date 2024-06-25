#include <stdio.h>
#include "hw1.h"


struct hashTable ppHashTable[HASH_TBL_SIZE];
struct list ppListHead[MAX_LIST_NUM];


void        Init(void)
{
    for(int i=0; i<HASH_TBL_SIZE; i++){
        CIRCLEQ_INIT(&ppHashTable[i]);
    }

    for(int i=0; i<MAX_LIST_NUM; i++){
        CIRCLEQ_INIT(&ppListHead[i]);
    }

}


void        InsertObjectToTail(Object* pObj, int objNum, List listNum)
{
    pObj->objnum = objNum;

    if(listNum != LIST3){
        int idx = objNum % HASH_TBL_SIZE;
        CIRCLEQ_INSERT_TAIL(&ppHashTable[idx], pObj, hash);
    }

    CIRCLEQ_INSERT_TAIL(&ppListHead[listNum], pObj, link);
}


void        InsertObjectToHead(Object* pObj, int objNum, List listNum)
{
    pObj->objnum = objNum;

    if(listNum != LIST3){
        int idx = objNum % HASH_TBL_SIZE;
        CIRCLEQ_INSERT_HEAD(&ppHashTable[idx], pObj, hash);
    }

    if(listNum == LIST3){
        CIRCLEQ_INSERT_HEAD(&ppListHead[listNum], pObj, link);
    }
    else{
        CIRCLEQ_INSERT_TAIL(&ppListHead[listNum], pObj, link);
    }

}


Object*     FindObjectByNum(int objnum)
{
    for(int num=0; num<=2; num++){
        Object* iter = CIRCLEQ_FIRST(&ppListHead[num]);

        while(iter != (void*) &ppListHead[num]){
            if(iter->objnum == objnum){
                return iter;
            }

            iter = CIRCLEQ_NEXT(iter, link);
        }
    }

    return NULL;
}


BOOL        DeleteObject(Object* pObj)
{   
    Object* foundObj = FindObjectByNum(pObj->objnum);
    if(foundObj == NULL){
        return FALSE;
    }

    List listNum;

    for(List num=LIST1; num<=LIST3; num++){
        Object* iter = CIRCLEQ_FIRST(&ppListHead[num]);

        while(iter != (void*) &ppListHead[num]){
            if(iter->objnum == pObj->objnum){
                listNum = num;
            }

            iter = CIRCLEQ_NEXT(iter, link);
        }
    }
    
    CIRCLEQ_REMOVE(&ppListHead[listNum], foundObj, link);

    if(listNum != LIST3){
        int idx = (foundObj->objnum) % HASH_TBL_SIZE;
        CIRCLEQ_REMOVE(&ppHashTable[idx], foundObj, hash);
    }

    return TRUE;
}


BOOL        DeleteObjectByNum(int objNum)
{
    Object* foundObj = FindObjectByNum(objNum);
    if(foundObj == NULL){
        return FALSE;
    }

    List listNum;

     for(List num=LIST1; num<=LIST3; num++){
        Object* iter = CIRCLEQ_FIRST(&ppListHead[num]);

        while(iter != (void*) &ppListHead[num]){
            if(iter->objnum == objNum){
                listNum = num;
            }

            iter = CIRCLEQ_NEXT(iter, link);
        }
    }

    CIRCLEQ_REMOVE(&ppListHead[listNum], foundObj, link);

    if(listNum != LIST3){
        int idx = objNum % HASH_TBL_SIZE;
        CIRCLEQ_REMOVE(&ppHashTable[idx], foundObj, hash);
    }
    
    return TRUE;
}


int         EnumberateObjectsByListNum(List listnum, Object* ppObject[], int count)
{
    static Object* retObjects[100];
    
    Object* iter = CIRCLEQ_FIRST(&ppListHead[listnum]);

    int cnt = 0;
    while(iter != (void*) &ppListHead[listnum]){
        retObjects[cnt++] = iter; 
        iter = CIRCLEQ_NEXT(iter, link);
    }
    
    for(int i=0; i<cnt; i++){
        ppObject[i] = retObjects[i];
    }

    return cnt;

}

int         EnumberateObjectsByHashIndex(int hashnum, Object* ppObject[], int count)
{
    static Object* retObjects[100];
    
    Object* iter = CIRCLEQ_FIRST(&ppHashTable[hashnum]);

    int cnt = 0;
    while(iter != (void*) &ppHashTable[hashnum]){
        retObjects[cnt++] = iter; 
        iter = CIRCLEQ_NEXT(iter, hash);
    }
    
    for(int i=0; i<cnt; i++){
        ppObject[i] = retObjects[i];
    }

    return cnt;

}
