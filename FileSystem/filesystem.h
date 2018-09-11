#ifndef FILESYSTEM_FILESYSTEM_H
#define FILESYSTEM_FILESYSTEM_H

#define BLOCK_SIZE 512
#define DRIVE_SIZE 10485760 // 10 MB
#define NUM_BLOCKS (DRIVE_SIZE / (BLOCK_SIZE + sizeof(int)))

typedef enum {
    file, dir
} fileOrDir;

typedef struct {
    char name[20];
    int start;
    int fileSize;
    fileOrDir type: 1;
    int isOpen: 1;
} entry;

entry NULLENT = {0};
entry ROOT = {
        .type = dir,
        .start = 0,
};

typedef union {
    char file[BLOCK_SIZE];
    entry dir[BLOCK_SIZE / sizeof(entry)];
} block;

typedef struct {
    int FAT[NUM_BLOCKS];
    block data[NUM_BLOCKS];
} drive;

typedef struct {
    entry *meta;
    int ptr;
    int currentBlock;
} filePointer;

entry *getMeta(char *, drive *);
void createFile(char *, fileOrDir, drive *);
void writeFile(filePointer *, char *, int, drive *);
filePointer *openFile (char *, drive *);
void readFile(char *, filePointer *, int, drive *);
void closeFile(filePointer *);
void deleteFile(char *, drive *);
void rewindFile(filePointer *);
int allocateEmptyBlock(drive *);
void formatDrive(drive *);

#endif //FILESYSTEM_FILESYSTEM_H
