#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include "filesystem.h"

int main(int argc, const char **argv) {

    int fdes = open("drive", O_RDWR);
    drive *D = mmap(0, sizeof(drive), PROT_READ | PROT_WRITE, MAP_SHARED, fdes, 0);
    close(fdes);

    // Format drive
    //formatDrive(D);

    // Create files and directories
    createFile("/dir", dir, D);
    createFile("/dir/blah.txt", file, D);
    createFile("/dir/file2.txt", file, D);

    char *cats = "cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool cats are cool\n";

    // Write to blah.txt
    filePointer *f = openFile("/dir/blah.txt", D);
    writeFile(f, cats, strlen(cats), D);
    closeFile(f);

    // Write to file2.txt
    f = openFile("/dir/file2.txt", D);
    writeFile(f, "Sample Text", 12, D);
    closeFile(f);

    // Read from file2.txt
    /*f = openFile("/dir/file2.txt", D);
    char output[20];
    readFile(output, f, 12, D);
    closeFile(f);
    printf("%s\n", output);*/

    // Read from blah.txt
    char output[strlen(cats)];
    f = openFile("/dir/blah.txt", D);
    readFile(output, f, strlen(cats), D);
    closeFile(f);
    printf("%s\n", output);

    // Delete blah.txt
    deleteFile("/dir/blah.txt", D);
    if (openFile("/dir/blah.txt", D) == NULL) {
        printf("blah.txt was deleted!\n");
    } else {
        printf("blah.txt not delted. :(\n");
    }

    munmap(D, sizeof(drive));
    return 0;
}

// Returns a pointer to the metadata of the file on the drive
entry *getMeta(char *filepath, drive *D) {
    if (strcmp(filepath, "") == 0) {
        // return meta of root directory
        return &ROOT;
    }
    //create a pointer to a null entry
    entry *meta = &NULLENT;
    // Initialize loop variables
    int found = 0;
    fileOrDir last = dir;
    // Split of one token at a time
    for (char *token = strtok(filepath, "/"); token != NULL; token = strtok(NULL, "/")) {
        // Can't enter a file, return NULL
        if (last == file) {
            return NULL;
        }
        // Traverse current directory
        for (int traverseBlock = meta->start; traverseBlock != -1 && !found; traverseBlock = D->FAT[traverseBlock]) {
            for (int i = 0; i < BLOCK_SIZE / sizeof(entry) && !found; i++) {
                if (strcmp(D->data[traverseBlock].dir[i].name, token) == 0) {
                    // Name matches, continue search
                    last = D->data[traverseBlock].dir[i].type;
                    meta = &(D->data[traverseBlock].dir[i]);
                    found = 1;
                }
            }
        }
        // Current token not found, file doesn't exist
        if (!found) {
            return NULL;
        }
        found = 0;
    }
    // File was never found, return NULL
    if (meta == &NULLENT) {
        return NULL;
    }
    // Return the last pointer found
    return meta;
}

// Create a file at the given path
void createFile(char *filepath, fileOrDir type, drive *D) {
    // Copy the filepath into a mutable array
    char newPathStr[strlen(filepath)];
    strcpy(newPathStr, filepath);
    // Check if file already exists
    if (getMeta(newPathStr, D) != NULL) {
        return;
    }
    // Restore filepath
    strcpy(newPathStr, filepath);
    // Split name of new file into new variable
    char *name = strrchr(newPathStr, '/');
    if (name == NULL) {
        // All filepaths should start with '/', so this should not be null. If it is, just return
        return;
    } else {
        *name = '\0';
        name = name + 1;
    }
    // Ensure name is appropriate length
    if (strlen(name) > sizeof(((drive){0}).data->dir->name)) {
        return;
    }
    // Find start of directory
    int dirStart;
    entry *dirMeta = getMeta(newPathStr, D);
    // Ensure that directory exists
    if (dirMeta == NULL) {
        return;
    } else {
        // Directory exists, so set the start block appropriately
        dirStart = dirMeta->start;
    }
    // Allocate block for the new file
    int fileStart = allocateEmptyBlock(D);
    if (fileStart == -1) {
        // No block was successfully allocated, so don't create the file
        return;
    }

    // Find first empty slot in directory, and fill it in
    for (; dirStart != -1; dirStart = D->FAT[dirStart]) {
        for (int i = 0; i < BLOCK_SIZE / sizeof(entry); i++) {
            if (D->data[dirStart].dir[i].name[0] == '\0') {
                D->data[dirStart].dir[i].start = fileStart;
                D->data[dirStart].dir[i].fileSize = 0;
                D->data[dirStart].dir[i].type = type;
                strcpy(D->data[dirStart].dir[i].name, name);
                return;
            }
        }
    }
    // File was not crated, so add a new block to the directory and fill then add the file there
    int newDirBlock = allocateEmptyBlock(D);
    if (newDirBlock == -1) {
        // No block was successfully allocated, so don't create the file
        return;
    }
    // Fill in the file in the new block
    D->FAT[dirStart] = newDirBlock;
    D->data[newDirBlock].dir[0].start = fileStart;
    D->data[newDirBlock].dir[0].fileSize = 0;
    D->data[newDirBlock].dir[0].type = type;
    strcpy(D->data[newDirBlock].dir[0].name, name);
}

// Write data to a file
void writeFile(filePointer *file, char *data, int len, drive *D) {
    // Amount of data already written
    int offset = 0;
    // Amount to be written to this block
    int amount = file->ptr + len > BLOCK_SIZE ? BLOCK_SIZE - file->ptr : len;
    // Loop while there is data left to write
    while (len > 0) {
        // Copy current amount into data block
        memcpy(&(D->data[file->currentBlock].file[file->ptr]), data + offset, amount);
        // Subtract length from amount
        len -= amount;
        // Move file pointer ahead
        file->ptr += amount;
        // Increase the offset
        offset += amount;
        // Increase the file size
        file->meta->fileSize += amount;
        // Check of end of block was reached
        if (file->ptr == BLOCK_SIZE) {
            // Add a new block
            D->FAT[file->currentBlock] = allocateEmptyBlock(D);
            // Move to that block
            file->currentBlock = D->FAT[file->currentBlock];
            // Reset pointer to beginning of block
            file->ptr = 0;
        }
        // Calculate amount to be written next
        amount = len > BLOCK_SIZE ? BLOCK_SIZE : len;
    }
}

// Open a file
filePointer *openFile(char *filepath, drive *D) {
    // Create mutable copy of filepath
    char newFilepath[strlen(filepath)];
    strcpy(newFilepath, filepath);
    // Create pointer for file
    entry *meta = getMeta(newFilepath, D);
    // If file doesn't exist or if file is already open, it can't be opened
    if (meta == NULL || meta->isOpen) {
        return NULL;
    }
    // Create filePointer
    filePointer *file = (filePointer *)malloc(sizeof(filePointer));
    file->meta = meta;
    file->ptr = 0;
    file->currentBlock = file->meta->start;
    file->meta->isOpen = 1;
    // return filePointer
    return file;
}

// Read data from a file
void readFile(char *dest, filePointer *file, int len, drive *D) {
    // Amount of data already read
    int offset = 0;
    // clear the destination to make room for the data to be read
    memset(dest, 0, len);
    // Calculate amount of data to be read
    int amount = file->ptr + len> BLOCK_SIZE ? BLOCK_SIZE - (file->ptr) : len;
    // Loop through the blocks of the file
    while (len > 0 && file->currentBlock != -1) {
        // Copy data into the destination array
        memcpy(dest + offset, &(D->data[file->currentBlock].file[file->ptr]), amount);
        // Increase offset
        offset += amount;
        // Decrease amount left to be read
        len -= amount;
        // Increase file pointer
        file->ptr += amount;
        // Check if the end of the block has been reached
        if (file->ptr == BLOCK_SIZE) {
            // Reset file pointer
            file->ptr = 0;
            // Move to next block
            file->currentBlock = D->FAT[file->currentBlock];
        }
        // Calculate amount to be read from next block
        amount = len > BLOCK_SIZE ? BLOCK_SIZE : len;
    }
}

// Close a file
void closeFile(filePointer *file) {
    // Set the file to closed
    file->meta->isOpen = 0;
    // Delete the pointer
    free(file);
}

// Delete a file
void deleteFile(char *filepath, drive *D) {
    // Create a mutable copy of the filepath
    char newFilePath[strlen(filepath)];
    strcpy(newFilePath, filepath);
    // Get the file's metadata
    entry *file = getMeta(newFilePath, D);
    // If the file doesn't exist or is open, don't try to delete it
    if (file == NULL || file->isOpen) {
        return;
    }
    // Check if the filepath points to a directory
    if (file->type == dir) {
        // Ensure the directory is empty before deleting
        for (int block = file->start; block != -1; block = D->FAT[block]){
            for (int i = 0; i < BLOCK_SIZE / sizeof(entry); i++) {
                if (D->data[block].dir[i].name != '\0') {
                    return;
                }
            }
        }
    }
    // Set all blocks in the file to free
    for (int block = file->start, nextBlock = -1; block != -1; block = nextBlock) {
        nextBlock = D->FAT[block];
        D->FAT[block] = 0;
    }
    // Delete the file's entry
    memset(file, 0, sizeof(entry));
}

// Move the file's pointer back to the beginning of the file
void rewindFile(filePointer *file) {
    // Set block to the first in the file
    file->currentBlock = file->meta->start;
    // Set poitner to the start of the block
    file->ptr = 0;
}

// Allocate a new block
int allocateEmptyBlock(drive *D) {
    // Find the first empty block
    for (int i = 1; i < NUM_BLOCKS; i++) {
        if (D->FAT[i] == 0) {
            // Set mark the block as occupied
            D->FAT[i] = -1;
            // Return the block number
            return i;
        }
    }
    // All blocks full, return invalid block
    return -1;
}

// Wipe all data from a drive
void formatDrive(drive *D) {
    // Set all bytes to 0
    memset(D, 0, sizeof(drive));
    // Mark the start of the root directory
    D->FAT[0] = -1;
}