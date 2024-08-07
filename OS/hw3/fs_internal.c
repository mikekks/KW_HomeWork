#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buf.h"
#include "fs.h"

void SetInodeBitmap(int inodeno) {
    char* pData = malloc(BLOCK_SIZE);
    BufRead(INODE_BITMAP_BLOCK_NUM, pData);

    int byteIndex = inodeno / 8;
    int bitIndex = 7 - (inodeno % 8);
    pData[byteIndex] |= (1 << bitIndex);

    BufWrite(INODE_BITMAP_BLOCK_NUM, pData);
}

void ResetInodeBitmap(int inodeno) {
    char *pData = malloc(BLOCK_SIZE);
    BufRead(INODE_BITMAP_BLOCK_NUM, pData);

    int byteIndex = inodeno / 8;
    int bitIndex = 7 - (inodeno % 8);
    pData[byteIndex] &= ~(1 << bitIndex);

    BufWrite(INODE_BITMAP_BLOCK_NUM, pData);
}

void SetBlockBitmap(int blkno)
{
    char *pData = malloc(BLOCK_SIZE);
    BufRead(BLOCK_BITMAP_BLOCK_NUM, pData);

    int byteIndex = blkno / 8;
    int bitIndex = 7 - (blkno % 8);
    pData[byteIndex] |= (1 << bitIndex);

    BufWrite(BLOCK_BITMAP_BLOCK_NUM, pData);
}

void ResetBlockBitmap(int blkno) {
    // Map 변경하기 전에, Block을 디스크에서 읽는 이유? -> 디스크에 있는 원본 Block 없이 설정해서 저장하면 원본 Block의 데이터가 손실된다??
    char *pData = malloc(BLOCK_SIZE);
    BufRead(BLOCK_BITMAP_BLOCK_NUM, pData);

    int byteIndex = blkno / 8;
    int bitIndex = 7 - (blkno % 8);
    pData[byteIndex] &= ~(1 << bitIndex);

    BufWrite(BLOCK_BITMAP_BLOCK_NUM, pData);
}

void PutInode(int inodeno, Inode* pInode)
{
    Inode *pData = malloc(BLOCK_SIZE);
    int blkno = (inodeno / NUM_OF_INODE_PER_BLOCK) + INODELIST_BLOCK_FIRST;
    int idx = inodeno % NUM_OF_INODE_PER_BLOCK;

    BufRead(blkno, pData);
    memcpy(&pData[idx], pInode, sizeof(Inode));
    BufWrite(blkno, pData);
}


void GetInode(int inodeno, Inode* pInode)
{
    Inode *pData = malloc(BLOCK_SIZE);
    int blkno = (inodeno / NUM_OF_INODE_PER_BLOCK) + INODELIST_BLOCK_FIRST;
    int idx = inodeno % NUM_OF_INODE_PER_BLOCK;

    BufRead(blkno, pData);
    memcpy(pInode, &pData[idx], sizeof(Inode));
}


int GetFreeInodeNum(void)
{
    char *pData = malloc(BLOCK_SIZE);
    BufRead(INODE_BITMAP_BLOCK_NUM, pData);

    for (int byteIdx = 0; byteIdx < 8; byteIdx++) {
        for (int bitIdx = 0; bitIdx < 8; bitIdx++) {
            int actualBitIdx = 7 - bitIdx;
            if ((pData[byteIdx] & (1 << actualBitIdx)) == 0) {
                return byteIdx * 8 + bitIdx;
            }
        }
    }

    return -1;
}


int GetFreeBlockNum(void)
{
    char *pData = malloc(BLOCK_SIZE);
    BufRead(BLOCK_BITMAP_BLOCK_NUM, pData);

    for (int byteIdx = 0; byteIdx < 64; byteIdx++) {
        for (int bitIdx = 0; bitIdx < 8; bitIdx++) {
            int actualBitIdx = 7 - bitIdx;
            if ((pData[byteIdx] & (1 << actualBitIdx)) == 0) {
                return byteIdx * 8 + bitIdx;
            }
        }
    }

    return -1;
}
