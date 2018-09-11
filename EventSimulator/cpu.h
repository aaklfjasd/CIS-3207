#include "globals.h"

int currentTime;

typedef struct {
    queue *queue;
    int processing;
    int currentPID;
} cpu;

cpu *initCPU();
void handleEventCPU(cpu *, event *, heap *);
void addProcessCPU(cpu *, event *, heap*);
void startProcessCPU(cpu *, heap *);
void finishProcessCPU(cpu *, heap*);