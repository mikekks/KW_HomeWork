#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buf.h"
#include "fs.h"

FileDesc pFileDesc[MAX_FD_ENTRY_MAX];
FileSysInfo* pFileSysInfo;
int getDirPos(const char *szDirName, int *curInodeNo, Inode *curInode,
              char* retToken);
void setRoot();

void InitFileSys() {

    for (int i = 0; i < 512; i++) {
        char *pData = malloc(sizeof(BLOCK_SIZE));
        BufWrite(i, pData);
    }
}

int     CreateFile(const char* szFileName)
{
    int inodeNo = GetFreeInodeNum();

    // 디렉토리 찾기(2,3번 동작 필요) - 시작
    char *token = malloc(100);
    int parentInodeNo = 0;
    Inode *parentInode = malloc(BLOCK_SIZE);

    getDirPos(szFileName, &parentInodeNo, parentInode, token);
    GetInode(parentInodeNo, parentInode); // 찾았을 경우 N-1 디렉토리 의미
    //printf("TEST TOKEN %s, %d   ", token, parentInodeNo);
    BOOL isAllocated = FALSE;
    for (int i = 0; i < NUM_OF_DIRECT_BLOCK_PTR; i++) {
        if (parentInode->dirBlockPtr[i] != 0) {
            DirEntry entries[NUM_OF_DIRENT_PER_BLOCK];
            BufRead(parentInode->dirBlockPtr[i], entries);

            for (int j = 0; j < NUM_OF_DIRENT_PER_BLOCK; j++) {
                if (entries[j].name[0] == '\0') {
                    //printf("  created BlkNo %d  \n", parentInode->dirBlockPtr[i]);
                    entries[j].inodeNum = inodeNo;
                    strncpy(entries[j].name, token, MAX_NAME_LEN);
                    BufWrite(parentInode->dirBlockPtr[i], entries);
                    isAllocated = TRUE;
                    break;
                }
            }
        }

        if (isAllocated) {
            break;
        }
    }

    if (!isAllocated) {
        printf("TEST CREATE FILE FAIL\n");
        return -1;
    }
    // 디렉토리 찾기(2,3번 동작 필요) - 끝

    Inode *pInode = malloc(sizeof(Inode));
    GetInode(inodeNo, pInode);
    //printf("   TEST CREATE FILE %d %d\n", inodeNo, pInode->dirBlockPtr[0]);
    pInode->type = FILE_TYPE_FILE;
    pInode->size = 0;
    pInode->allocBlocks = 0;
    PutInode(inodeNo, pInode);

    SetInodeBitmap(inodeNo);

    pFileSysInfo->numAllocInodes++;
    BufWrite(FILESYS_INFO_BLOCK, pFileSysInfo);

    for (int i = 0; i < MAX_FD_ENTRY_MAX; i++){
        if(pFileDesc[i].bUsed == 0){
            File* file = malloc(sizeof(File));
            file->inodeNum = inodeNo;
            file->fileOffset = 0;  // 0으로 해도 되나?
            pFileDesc[i].bUsed = 1;
            pFileDesc[i].pOpenFile = file;
            return i;
        }
    }

    return -1;
}

int     OpenFile(const char* szFileName)
{
    // 디렉토리 찾기(1,2번 동작 필요) - 시작
    char *token = malloc(100);
    int parentInodeNo = 0;
    Inode *parentInode = malloc(BLOCK_SIZE);

    if(getDirPos(szFileName, &parentInodeNo, parentInode, token) == -1){
        return -1;
    }
    GetInode(parentInodeNo, parentInode);

    int fileInodeNo = -1;
    BOOL isFound = FALSE;
    for (int i = 0; i < NUM_OF_DIRECT_BLOCK_PTR; i++) {
        if (parentInode->dirBlockPtr[i] != 0) {
            DirEntry entries[NUM_OF_DIRENT_PER_BLOCK];
            BufRead(parentInode->dirBlockPtr[i], entries);

            for (int j = 0; j < NUM_OF_DIRENT_PER_BLOCK; j++) {
                if (strcmp(entries[j].name, token) == 0) {
                    fileInodeNo = entries[j].inodeNum;
                    isFound = TRUE;
                    //printf("OPEN FILE HIT\n");
                    break;
                }
            }

            if (isFound) {
                break;
            }
        }
    }

    if (!isFound) {
        printf("TEST OPEN FILE FAIL\n");
        return -1;
    }

    // 디렉토리 찾기(1,2번 동작 필요) - 끝

    for (int i = 0; i < MAX_FD_ENTRY_MAX; i++) {
        if (!pFileDesc[i].bUsed) {
            File *file = malloc(sizeof(File));
            file->inodeNum = fileInodeNo;
            file->fileOffset = 0; // 0으로 해도 되나?
            pFileDesc[i].bUsed = 1;
            pFileDesc[i].pOpenFile = file;
            return i;
        }
    }

    return -1;
}


int     WriteFile(int fileDesc, char* pBuffer, int length)
{
    File *file = pFileDesc[fileDesc].pOpenFile;
    Inode *pInode = malloc(sizeof(Inode));
    GetInode(file->inodeNum, pInode);

    int totalAllocCount = ((length - 1) / BLOCK_SIZE) + 1;
    int curIdx = file->fileOffset / BLOCK_SIZE;
    int needBlockCount = 0;
    int blkNoList[BLOCK_SIZE] = {0, };

    if(curIdx + totalAllocCount > NUM_OF_DIRECT_BLOCK_PTR){
        return -1;
    }

    for (int i = curIdx; i < curIdx + totalAllocCount; i++) {
        int curBlkNo = pInode->dirBlockPtr[i];
        if (curBlkNo == 0) {
            needBlockCount++;
            curBlkNo = GetFreeBlockNum();
            SetBlockBitmap(curBlkNo);

            pInode->allocBlocks += 1;
            pInode->size += BLOCK_SIZE;
            pInode->dirBlockPtr[i] = curBlkNo;
        }

        Buf *pBuf = malloc(BLOCK_SIZE);
        memcpy(pBuf, pBuffer + (BLOCK_SIZE * i), BLOCK_SIZE);
        BufWrite(curBlkNo, pBuf);
    }

    PutInode(file->inodeNum, pInode);

    pFileSysInfo->numAllocBlocks += needBlockCount;
    pFileSysInfo->numFreeBlocks -= needBlockCount;
    BufWrite(FILESYS_INFO_BLOCK, pFileSysInfo);

    file->fileOffset += BLOCK_SIZE * totalAllocCount; // 더블 체크 필요.

    return BLOCK_SIZE * totalAllocCount;
}

int     ReadFile(int fileDesc, char* pBuffer, int length)
{
    File *file = pFileDesc[fileDesc].pOpenFile;
    Inode *pInode = malloc(sizeof(Inode));
    GetInode(file->inodeNum, pInode);

    int logicalBlkNo = (file->fileOffset) / BLOCK_SIZE;
    char *pData = malloc(BLOCK_SIZE);
    BufRead(pInode->dirBlockPtr[logicalBlkNo], pData);

    file->fileOffset += BLOCK_SIZE;  // 이 코드 맞나??

    memcpy(pBuffer, pData, length);

    return BLOCK_SIZE;
}


int     CloseFile(int fileDesc)
{
    free(pFileDesc[fileDesc].pOpenFile);
    pFileDesc[fileDesc].bUsed = 0;
}

int     RemoveFile(const char* szFileName)
{
    char *token = malloc(100);
    int parentInodeNo = 0;
    Inode *parentInode = malloc(BLOCK_SIZE);

    getDirPos(szFileName, &parentInodeNo, parentInode, token);
    GetInode(parentInodeNo, parentInode);

    int fileInodeNo = -1;
    BOOL isFound = FALSE;
    for (int i = 0; i < NUM_OF_DIRECT_BLOCK_PTR; i++) {
        if (parentInode->dirBlockPtr[i] != 0) {
            DirEntry entries[NUM_OF_DIRENT_PER_BLOCK];
            BufRead(parentInode->dirBlockPtr[i], entries);

            for (int j = 0; j < NUM_OF_DIRENT_PER_BLOCK; j++) {
                if (strcmp(entries[j].name, token) == 0) {
                    fileInodeNo = entries[j].inodeNum;
                    entries[j].inodeNum = 0;
                    entries[j].name[0] = '\0';
                    BufWrite(parentInode->dirBlockPtr[i], entries);
                    isFound = TRUE;
                    //printf("REMOVE FILE HIT\n");
                    break;
                }
            }

            if (isFound) {
                break;
            }
        }
    }

    if (!isFound) {
        printf("TEST REMOVE FILE FAIL\n");
        return -1;
    }

    Inode *fileInode = malloc(BLOCK_SIZE);
    GetInode(fileInodeNo, fileInode);

    for (int i = 0; i < NUM_OF_DIRECT_BLOCK_PTR; i++) {
        if (fileInode->dirBlockPtr[i] != 0) {
            ResetBlockBitmap(fileInode->dirBlockPtr[i]);
            fileInode->dirBlockPtr[i] = 0;
        }
    }
    fileInode->allocBlocks = 0;
    fileInode->size = 0;
    PutInode(fileInodeNo, fileInode);
    ResetInodeBitmap(fileInodeNo);

    // pFileSysInfo 업데이트
    pFileSysInfo->numAllocInodes--;
    pFileSysInfo->numAllocBlocks -= fileInode->allocBlocks;
    pFileSysInfo->numFreeBlocks += fileInode->allocBlocks;
    BufWrite(FILESYS_INFO_BLOCK, pFileSysInfo);

    // 메모리 해제
    free(fileInode);
    // free(parentInode);
    free(token);

    return 1;
}


int     MakeDir(const char* szDirName)
{
    int inodeNo = GetFreeInodeNum();
    //printf("%d\n", inodeNo);
    // 2,3번 동작 필요 - 시작

    char* token = malloc(100);
    int curInodeNo = 0;
    Inode* curInode = malloc(BLOCK_SIZE);

    getDirPos(szDirName, &curInodeNo, curInode, token);
    GetInode(curInodeNo, curInode); // 찾았을 경우 N-1 디렉토리 의미
    //printf("TEST TOKEN %s, %d   ", token, curInodeNo);
    BOOL isAllocated = FALSE;
    for (int i = 0; i < NUM_OF_DIRECT_BLOCK_PTR; i++) {
        if (curInode->dirBlockPtr[i] != 0) {
            DirEntry entries[NUM_OF_DIRENT_PER_BLOCK];
            BufRead(curInode->dirBlockPtr[i], entries);

            for (int j = 0; j < NUM_OF_DIRENT_PER_BLOCK; j++) {
                if (entries[j].name[0] == '\0') {
                    //printf("  BlkNo %d  %d  ", curInode->dirBlockPtr[i], j);
                    entries[j].inodeNum = inodeNo;
                    strncpy(entries[j].name, token, MAX_NAME_LEN);
                    BufWrite(curInode->dirBlockPtr[i], entries);
                    isAllocated = TRUE;
                    break;
                }
            }
        }
        else{
            // 새로운 블록 할당 필요
            int tmpBlkNo = GetFreeBlockNum();

            DirEntry *entries = malloc(sizeof(BLOCK_SIZE));
            entries[0].inodeNum = inodeNo;
            strncpy(entries[0].name, token, MAX_NAME_LEN);
            BufWrite(tmpBlkNo, entries);

            curInode->allocBlocks++;
            curInode->size += BLOCK_SIZE;
            curInode->dirBlockPtr[i] = tmpBlkNo;

            PutInode(curInodeNo, curInode);

            SetBlockBitmap(tmpBlkNo);
            isAllocated = TRUE;
        }

        if (isAllocated) {
            break;
        }
    }

    if(!isAllocated){
        printf("test: Fail\n");
        return -1;
    }

    int blkNo = GetFreeBlockNum();
    //printf(" ||   %s %d %d \n ", szDirName, blkNo, inodeNo);

    DirEntry *cData = malloc(BLOCK_SIZE);
    cData[0].inodeNum = inodeNo;
    cData[0].name[0] = '.';
    cData[0].name[1] = '\0';

    cData[1].inodeNum = curInodeNo;
    cData[1].name[0] = '.';
    cData[1].name[1] = '.';
    cData[1].name[2] = '\0';
    BufWrite(blkNo, cData);

    // 2,3번 동작 필요 - 끝

    Inode *pInode = malloc(sizeof(Inode));
    GetInode(inodeNo, pInode);
    pInode->dirBlockPtr[0] = blkNo;
    pInode->dirBlockPtr[1] = 0;
    pInode->dirBlockPtr[2] = 0;
    pInode->dirBlockPtr[3] = 0;
    pInode->dirBlockPtr[4] = 0;
    pInode->type = FILE_TYPE_DIR;
    pInode->size = BLOCK_SIZE;
    pInode->allocBlocks = 1;
    PutInode(inodeNo, pInode);

    SetBlockBitmap(blkNo);
    SetInodeBitmap(inodeNo);

    pFileSysInfo->numAllocBlocks++;
    pFileSysInfo->numFreeBlocks--;
    pFileSysInfo->numAllocInodes++;
    BufWrite(FILESYS_INFO_BLOCK, pFileSysInfo);

}


int     RemoveDir(const char* szDirName)
{
    // MakeDir 역동작
    int curInodeNo = 0;
    int parentInodeNo = 0;
    Inode *parentInode = malloc(BLOCK_SIZE);
    Inode *curInode = malloc(BLOCK_SIZE);
    char *token = malloc(100);

    getDirPos(szDirName, &parentInodeNo, parentInode, token);
    GetInode(parentInodeNo, parentInode);
   // printf("TEST REMOVE %d %s", parentInodeNo, token);

    // 자식(지우려는 대상) 찾기
    BOOL isFound = FALSE;
    for (int i = 0; i < NUM_OF_DIRECT_BLOCK_PTR; i++) {
        if (parentInode->dirBlockPtr[i] != 0) {
            DirEntry entries[NUM_OF_DIRENT_PER_BLOCK];
            BufRead(parentInode->dirBlockPtr[i], entries);

            for (int j = 0; j < NUM_OF_DIRENT_PER_BLOCK; j++) {
                if (strcmp(entries[j].name, token) == 0) {
                    GetInode(entries[j].inodeNum, curInode);
                    curInodeNo = entries[j].inodeNum;
                    isFound = TRUE;
                    break;
                }
            }
        }

        if(isFound) break;
    }

    if(!isFound)
        return -1;

    // 디렉토리가 비어 있는지 확인('.' 유무로 판단)
    for (int i = 0; i < NUM_OF_DIRECT_BLOCK_PTR; i++) {
        if (curInode->dirBlockPtr[i] != 0) {
            DirEntry entries[NUM_OF_DIRENT_PER_BLOCK];
            BufRead(curInode->dirBlockPtr[i], entries);

            for (int j = 0; j < NUM_OF_DIRENT_PER_BLOCK; j++) {
                //printf(" %s \n", entries[j].name);
                if (!(entries[j].name[0] == '.' || entries[j].name[0] == '\0')) {
                    printf(" REMOVE FAIL\n");
                    free(curInode);
                    free(token);
                    return -1;
                }
            }
        }
    }


    BOOL isRemoved = FALSE;
    for (int i = 0; i < NUM_OF_DIRECT_BLOCK_PTR; i++) {
        if (parentInode->dirBlockPtr[i] != 0) {
            DirEntry entries[NUM_OF_DIRENT_PER_BLOCK];
            BufRead(parentInode->dirBlockPtr[i], entries);

            for (int j = 0; j < NUM_OF_DIRENT_PER_BLOCK; j++) {
                if (entries[j].inodeNum == curInodeNo) {
                    entries[j].inodeNum = 0;
                    entries[j].name[0] = '\0';
                    BufWrite(parentInode->dirBlockPtr[i], entries);
                    isRemoved = TRUE;
                    //printf("  HIT  \n");

                    for (int k = 0; k < NUM_OF_DIRENT_PER_BLOCK; k++) {
                        if (entries[k].inodeNum != 0)
                            break;

                        if (k+1 == NUM_OF_DIRENT_PER_BLOCK){
                            // 블록 해제
                            ResetBlockBitmap(parentInode->dirBlockPtr[i]);
                            parentInode->dirBlockPtr[i] = 0;
                            parentInode->size -= BLOCK_SIZE;
                            parentInode->allocBlocks -= 1;
                            PutInode(parentInodeNo, parentInode);
                        }
                    }

                    break;
                    }
            }

            if (isRemoved) {
                break;
            }
        }
    }

    if (!isRemoved) {
        printf("Test FAIL\n");
        return -1;
    }


    for (int i = 0; i < NUM_OF_DIRECT_BLOCK_PTR; i++) {
        if (curInode->dirBlockPtr[i] != 0) {
            ResetBlockBitmap(curInode->dirBlockPtr[i]);
            curInode->dirBlockPtr[i] = 0;
        }
    }
    curInode->allocBlocks = 0;
    curInode->size = 0;
    PutInode(curInodeNo, curInode);
    // curInode->type
    ResetInodeBitmap(curInodeNo);


    // pFileSysInfo 업데이트
    pFileSysInfo->numAllocBlocks -= curInode->allocBlocks;
    pFileSysInfo->numFreeBlocks += curInode->allocBlocks;
    pFileSysInfo->numAllocInodes--;
    BufWrite(FILESYS_INFO_BLOCK, pFileSysInfo);

    free(curInode);
    free(token);
}

int   EnumerateDirStatus(const char* szDirName, DirEntryInfo* pDirEntry, int dirEntrys)
{
    int count = 0;

    char *token = malloc(100);
    int curInodeNo = 0;
    Inode *curInode = malloc(BLOCK_SIZE);

    getDirPos(szDirName, &curInodeNo, curInode, token);
    GetInode(curInodeNo, curInode);  // N-1 디렉토리
   // printf("TEST %s %d\n", token, curInodeNo);
    BOOL isFound = FALSE;
    for (int i = 0; i < NUM_OF_DIRECT_BLOCK_PTR; i++) {
        if (curInode->dirBlockPtr[i] != 0) {
            DirEntry entries[NUM_OF_DIRENT_PER_BLOCK];
            BufRead(curInode->dirBlockPtr[i], entries);

            for (int j = 0; j < NUM_OF_DIRENT_PER_BLOCK; j++) {
                //printf("%s\n", entries[j].name);
                if (strcmp(entries[j].name, token) == 0) {
                    Inode *tmpNode = malloc(sizeof(BLOCK_SIZE));
                    GetInode(entries[j].inodeNum, curInode);
                    isFound = TRUE;
                    break;
                }
            }
        }

        if(isFound) break;
    }

    if(!isFound){
        return -1;
    }


    for (int i = 0; i < NUM_OF_DIRECT_BLOCK_PTR; i++) {
        if (curInode->dirBlockPtr[i] != 0) {
            DirEntry entries[NUM_OF_DIRENT_PER_BLOCK];
            BufRead(curInode->dirBlockPtr[i], entries);

            for (int j = 0; j < NUM_OF_DIRENT_PER_BLOCK; j++) {
                //printf("%s\n", entries[j].name);
                if (entries[j].name[0] != '\0') {  // 이 조건 맞는지 더블체크 필요.
                    Inode* tmpNode = malloc(sizeof(BLOCK_SIZE));
                    pDirEntry[count].inodeNum = entries[j].inodeNum;
                    GetInode(entries[j].inodeNum, tmpNode);
                    pDirEntry[count].type = tmpNode->type;
                    strncpy(pDirEntry[count].name, entries[j].name, MAX_NAME_LEN);
                    count++;
                    if (count == dirEntrys){
                        return count;
                    }
                }
            }
        }
    }

    return count;
}


void    CreateFileSystem()
{
    // 파일시스템 초기화 단계기 때문에 Block0부터 Block511까지 0으로 초기화를 해야 한다(디스크 초기화).

    // 디스크 초기화 후에, 루트 디렉토리부터 생성하자. 그 후에 FileSysInfo를 초기화하자.

    // (1) Block Bitmap의 Bit 7 부터 검색하여 free block 검색. GetFreeBlockNum
    // 함수를 사용한다. Block 7라고 가정하자.
    // > Data region의 시작이 block 7이기 때문에 block 7부터 빈 공간을 찾도록
    // GetFreeBlockNum 함수를 구현하자

    // (2) Inode Bitmap의 Bit 0부터 검색하여 free inode 검색. GetFreeInodeNum
    // 함수를 사용한다. Inode 0라고 가정하자

    DevCreateDisk();
    BufInit();
    DevResetDiskAccessCount();

    InitFileSys();

    char *pData = malloc(BLOCK_SIZE);
    BufRead(BLOCK_BITMAP_BLOCK_NUM, pData);
    for (int i = 0; i < BLOCK_SIZE; i++) {
        pData[i] = 0;
    }
    BufRead(INODE_BITMAP_BLOCK_NUM, pData);
    for (int i = 0; i < BLOCK_SIZE; i++) {
        pData[i] = 0;
    }

    for (int i = 0; i <= 6; i++)
        SetBlockBitmap(i);

    for (int i = 7; i < BLOCK_SIZE; i++){
        ResetBlockBitmap(i);
    }

    setRoot();
}


void    OpenFileSystem()
{
    DevOpenDisk();
    BufInit();
    DevResetDiskAccessCount();

    // InitFileSys();
    for (int i = 0; i < 512; i++) {
        char *pData = malloc(sizeof(BLOCK_SIZE));
        BufRead(i, pData);
    }

    pFileSysInfo = malloc(sizeof(FileSysInfo));
    BufRead(FILESYS_INFO_BLOCK, pFileSysInfo);

    for (int i = 0; i < 6; i++)
        SetBlockBitmap(i);
}

void CloseFileSystem()
{
    BufSync();
    DevCloseDisk();
}

int GetFileStatus(const char* szPathName, FileStatus* pStatus)
{
    // 경로 찾기
    char *token = malloc(100);
    int curInodeNo = 0;
    Inode *curInode = malloc(BLOCK_SIZE);

    getDirPos(szPathName, &curInodeNo, curInode, token);
    GetInode(curInodeNo, curInode); // 찾았을 경우 N-1 디렉토리 의미

    // FileStatus 값
    BOOL isFound = FALSE;
    for (int i = 0; i < NUM_OF_DIRECT_BLOCK_PTR; i++) {
        if (curInode->dirBlockPtr[i] != 0) {
            DirEntry entries[NUM_OF_DIRENT_PER_BLOCK];
            BufRead(curInode->dirBlockPtr[i], entries);

            for (int j = 0; j < NUM_OF_DIRENT_PER_BLOCK; j++) {
                if (strcmp(entries[j].name, token) == 0) {
                    Inode *retInode = malloc(BLOCK_SIZE);
                    GetInode(entries[j].inodeNum, retInode);
                    pStatus->allocBlocks = retInode->allocBlocks;
                    pStatus->size = retInode->size;
                    pStatus->type = retInode->type;
                    pStatus->dirBlockPtr[0] = retInode->dirBlockPtr[0];
                    pStatus->dirBlockPtr[1] = retInode->dirBlockPtr[1];
                    pStatus->dirBlockPtr[2] = retInode->dirBlockPtr[2];
                    pStatus->dirBlockPtr[3] = retInode->dirBlockPtr[3];
                    pStatus->dirBlockPtr[4] = retInode->dirBlockPtr[4];
                    isFound = TRUE;
                    break;
                }
            }

            if (isFound) {
                break;
            }
        }
    }

    if (!isFound) {
        printf("test: Fail\n");
        return -1;
    }
}

void Sync()
{
    BufSync();
}

// 내부 함수들

int getDirPos(const char *szDirName, int* curInodeNo, Inode* curInode, char* retToken) {

    char* token;
    char* nextToken;
    char* delimiter = "/";
    *curInodeNo = 0;

    char path[100];
    strncpy(path, szDirName, sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';

    token = strtok(path, delimiter);
    if (token) {
        nextToken = strtok(NULL, delimiter);
    } else {
        return -1;
    }

    while (token != NULL) {
        if (nextToken == NULL) {
            sprintf(retToken, "%s", token);
            return 1;
        }

        GetInode(*curInodeNo, curInode);

        BOOL isFound = FALSE;
        for (int i = 0; i < NUM_OF_DIRECT_BLOCK_PTR; i++) {
            if (curInode->dirBlockPtr[i] != 0) {
                DirEntry entries[NUM_OF_DIRENT_PER_BLOCK];
                BufRead(curInode->dirBlockPtr[i], entries);

                for (int j = 0; j < NUM_OF_DIRENT_PER_BLOCK; j++) {
                    //printf("  %d DIR %s  ", *curInodeNo, entries[j].name);
                    if (strcmp(entries[j].name, token) == 0) {
                        *curInodeNo = entries[j].inodeNum;
                        isFound = TRUE;
                        break;
                    }
                }

                if (isFound) {
                    break;
                }
            }
        }

        if (!isFound) {
            return -1;
        }

        token = nextToken;
        nextToken = strtok(NULL, delimiter);
    }

    return 1;
}

void setRoot(){
    int blkNo = GetFreeBlockNum();
    int inodeNo = GetFreeInodeNum();

    DirEntry *pData = malloc(sizeof(BLOCK_SIZE));
    pData[0].inodeNum = inodeNo;
    pData[0].name[0] = '.';
    pData[0].name[1] = '\0';
    BufWrite(blkNo, pData);

    pFileSysInfo = malloc(sizeof(FileSysInfo));
    pFileSysInfo->numAllocBlocks++;
    pFileSysInfo->numFreeBlocks--;
    pFileSysInfo->numAllocInodes++;

    BufWrite(FILESYS_INFO_BLOCK, pFileSysInfo);

    Inode *pInode = malloc(sizeof(Inode));
    GetInode(0, pInode);
    pInode->dirBlockPtr[0] = blkNo;
    pInode->dirBlockPtr[1] = 0;
    pInode->dirBlockPtr[2] = 0;
    pInode->dirBlockPtr[3] = 0;
    pInode->dirBlockPtr[4] = 0;
    //printf("BLK %d\n", blkNo);
    pInode->type = FILE_TYPE_DIR;
    pInode->size = BLOCK_SIZE;
    pInode->allocBlocks = 1;
    PutInode(0, pInode);

    // 2개 다 해야겠지? 루트랑 시스템 info
    SetBlockBitmap(blkNo);
    SetInodeBitmap(inodeNo);
}
