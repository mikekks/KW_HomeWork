#include "disk.h"
#include "fs.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define FILENAME_MAX_LEN 100
#define DIR_NUM_MAX 100

void PrintInodeBitmap(void) {
    int i;
    char *pBitmap = malloc(BLOCK_SIZE);

    Sync();
    DevReadBlock(INODE_BITMAP_BLOCK_NUM, pBitmap);
    printf("Inode bitmap: ");
    for (i = 0; i < 64 / 8; i++) // 64 --> 8
        printf("%d", pBitmap[i]);
    printf("\n");
}

void PrintBlockBitmap(void) {
    int i;
    char *pBitmap = malloc(BLOCK_SIZE);

    Sync();
    DevReadBlock(BLOCK_BITMAP_BLOCK_NUM, pBitmap);
    printf("Block bitmap: ");
    for (i = 0; i < BLOCK_SIZE / 8; i++)
        printf("%d", pBitmap[i]);
    printf("\n");
}

void ListDirContents(const char *dirName) {
    int i;
    int count;
    DirEntryInfo pDirEntry[DIR_NUM_MAX];
    char outputName[FILENAME_MAX_LEN];
    FileStatus status;
    Inode pInode;

    count = EnumerateDirStatus(dirName, pDirEntry, DIR_NUM_MAX);

    printf("[%s]Sub-directory:\n", dirName);
    for (i = 0; i < count; i++) {
        memset(outputName, 0, FILENAME_MAX_LEN);
        GetInode(pDirEntry[i].inodeNum, &pInode);
        if (pDirEntry[i].type == FILE_TYPE_FILE) {
            printf("\t name:%s, inode no:%d, type:file, size:%d, blocks:%d\n",
                   pDirEntry[i].name, pDirEntry[i].inodeNum, pInode.size,
                   pInode.allocBlocks);
        } else if (pDirEntry[i].type == FILE_TYPE_DIR)
            printf(
                "\t name:%s, inode no:%d, type:directory size:%d, blocks:%d\n",
                pDirEntry[i].name, pDirEntry[i].inodeNum, pInode.size,
                pInode.allocBlocks);
        else {
            assert(0);
        }
        sprintf(outputName, "%s/%s", dirName, pDirEntry[i].name);
        GetFileStatus(outputName, &status);
        printf("\t name:%s, allocBlocks:%d, size:%d, dirBlockPtr:%d, %d, %d, "
               "%d, %d\n",
               pDirEntry[i].name, status.allocBlocks, status.size,
               status.dirBlockPtr[0], status.dirBlockPtr[1],
               status.dirBlockPtr[2], status.dirBlockPtr[3],
               status.dirBlockPtr[4]);
    }
}

void TestCase1(void) {
    int i;
    char dirName[MAX_NAME_LEN];

    printf(" ---- Test Case 1 ----\n");

    MakeDir("/tmp");
    MakeDir("/usr");
    MakeDir("/etc");
    MakeDir("/home");
    /* make home directory */
    for (i = 0; i < 5; i++) {
        memset(dirName, 0, MAX_NAME_LEN);
        sprintf(dirName, "/home/user%d", i);
        MakeDir(dirName);
    }
    /* make etc directory */
    for (i = 0; i < 34; i++) {
        memset(dirName, 0, MAX_NAME_LEN);
        sprintf(dirName, "/etc/dev%d", i);
        MakeDir(dirName);
    }

    ListDirContents("/home");
    ListDirContents("/etc");

    /* remove subdirectory of etc directory */
    for (i = 33; i >= 0; i--) {
        memset(dirName, 0, MAX_NAME_LEN);
        sprintf(dirName, "/etc/dev%d", i);
        RemoveDir(dirName);
    }

    ListDirContents("/etc");

    /* remove subdirectory of root directory except /home */
    RemoveDir("/etc");
    RemoveDir("/usr");
    RemoveDir("/tmp");
}

void TestCase2(void) {
    int i;
    char fileName[FILENAME_MAX_LEN];
    char tempName[FILENAME_MAX_LEN];
    char paths[15][FILENAME_MAX_LEN];
    int fd[4] = {
        0,
    };

    memset(fileName, 0, FILENAME_MAX_LEN);
    memset(tempName, 0, FILENAME_MAX_LEN);

    sprintf(fileName, "/dir");
    MakeDir(fileName);

    ListDirContents("/dir");
    for (i = 0; i < 15; i++) {
        sprintf(fileName, "%s/dir%d", fileName, i);
        memset(paths[i], 0, FILENAME_MAX_LEN);
        sprintf(paths[i], "%s", fileName);
        MakeDir(fileName);
    }

    for (i = 0; i < 4; i++) {
        sprintf(tempName, "%s", fileName);
        sprintf(tempName, "%s/%d.txt", tempName, i);
        fd[i] = CreateFile(tempName);
    }

    ListDirContents(fileName);

    for (i = 0; i < 4; i++) {
        sprintf(tempName, "%s", fileName);
        sprintf(tempName, "%s/%d.txt", tempName, i);
        CloseFile(fd[i]);
        RemoveFile(tempName);
    }

    for (i = 14; i >= 0; i--) {
        ListDirContents(paths[i]);
        RemoveDir(paths[i]);
    }
}

void TestCase3(void) {
    int i, j, k;
    char alphabet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123"
                      "456789!@#$^&*()_";
    char fileName[FILENAME_MAX_LEN];
    char *pBuffer1 = (char *)malloc(BLOCK_SIZE);
    char *pBuffer2 = (char *)malloc(BLOCK_SIZE);
    int cIndex = 0;
    int fd[4] = {
        0,
    };

    MakeDir("/home/test");
    for (i = 0; i < 4; i++) {
        memset(fileName, 0, FILENAME_MAX_LEN);
        sprintf(fileName, "/home/test/file%d", i);
        fd[i] = CreateFile(fileName);
    }

    for (i = 0; i < 5; i++) {
        for (j = 0; j < 4; j++) {
            char *str = (char *)malloc(BLOCK_SIZE);
            memset(str, 0, BLOCK_SIZE);
            for (k = 0; k < BLOCK_SIZE; k++)
                str[k] = alphabet[cIndex];
            WriteFile(fd[j], str, BLOCK_SIZE);
            cIndex++;
            free(str);
        }
    }

    for (i = 0; i < 4; i++)
        CloseFile(fd[i]);

    for (i = 0; i < 4; i++) {
        memset(fileName, 0, FILENAME_MAX_LEN);
        sprintf(fileName, "/home/test/file%d", i);
        fd[i] = OpenFile(fileName);
    }

    cIndex = 0;

    for (i = 0; i < 5; i++) {
        for (j = 0; j < 4; j++) {
            memset(pBuffer1, 0, BLOCK_SIZE);
            for (k = 0; k < BLOCK_SIZE; k++)
                pBuffer1[k] = alphabet[cIndex];
            memset(pBuffer2, 0, BLOCK_SIZE);
            ReadFile(fd[j], pBuffer2, BLOCK_SIZE);
            cIndex++;
        }
    }
}

void TestCase4(void) {
    int i, j;
    int fd;
    char fileName[FILENAME_MAX_LEN];
    char pBuffer[1024];
    char pBuffer1[BLOCK_SIZE];

    printf(" ---- Test Case 4 ----\n");
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 5; j++) {
            memset(fileName, 0, FILENAME_MAX_LEN);
            sprintf(fileName, "/home/user%d/file%d", i, j);
            fd = CreateFile(fileName);
            memset(pBuffer1, 0, BLOCK_SIZE);
            strcpy(pBuffer1, fileName);
            WriteFile(fd, pBuffer1, BLOCK_SIZE);
            CloseFile(fd);
        }
    }

    for (i = 0; i < 5; i++) {
        if (i % 2 == 0) {
            memset(fileName, 0, FILENAME_MAX_LEN);
            sprintf(fileName, "/home/user3/file%d", i);
            RemoveFile(fileName);
        }
    }
    printf(" ---- Test Case 4: files of even number removed ----\n");

    for (i = 0; i < 5; i++) {
        if (i % 2) {
            memset(fileName, 0, FILENAME_MAX_LEN);
            sprintf(fileName, "/home/user3/file%d", i);
            fd = OpenFile(fileName);

            memset(pBuffer, 0, 1024);
            strcpy(pBuffer, fileName);
            WriteFile(fd, pBuffer, 513);
            CloseFile(fd);
        }
    }
    printf(" ---- Test Case 4: files of odd number overwritten ----\n");

    ListDirContents("/home/user3");

    for (i = 0; i < 5; i++) {
        if (i % 2 == 0) {
            memset(fileName, 0, FILENAME_MAX_LEN);
            sprintf(fileName, "/home/user3/file%d", i);
            fd = CreateFile(fileName);

            memset(pBuffer, 0, 1024);
            strcpy(pBuffer, fileName);
            WriteFile(fd, pBuffer, 513);
            WriteFile(fd, pBuffer, 513);
            CloseFile(fd);
        }
    }
    printf(
        " ---- Test Case 4: files of even number re-created & written ----\n");

    ListDirContents("/home/user3");
}

int main(int argc, char **argv) {
    int TcNum;
    if (argc < 3) {
    ERROR:
        printf("usage: a.out [format | readwrite] [1-4])\n");
        return -1;
    }
    if (strcmp(argv[1], "format") == 0)
        CreateFileSystem();
    else if (strcmp(argv[1], "readwrite") == 0)
        OpenFileSystem();
    else
        goto ERROR;

    TcNum = atoi(argv[2]);

    switch (TcNum) {
    case 1:
        TestCase1();
        PrintInodeBitmap();
        PrintBlockBitmap();
        break;
    case 2:
        TestCase2();
        PrintInodeBitmap();
        PrintBlockBitmap();
        break;
    case 3:
        TestCase3();
        PrintInodeBitmap();
        PrintBlockBitmap();
        break;
    case 4:
        TestCase4();
        PrintInodeBitmap();
        PrintBlockBitmap();
        break;

    default:
        CloseFileSystem();
        goto ERROR;
    }

    CloseFileSystem();

    return 0;
}
