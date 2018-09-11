#include "globals.h"

typedef struct {
    queue *queue;
    int processing;
} disk;

disk *initDisk();
void handleEventDisk(disk *, event *, heap *);
void addProcessDisk(disk *, event *, heap *);
void startProcessDisk(disk *, heap *);
void finishProcessDisk(disk *, heap*);

disk *findShortestQueue(disk *, disk *);