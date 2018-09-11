#include "disk.h"
#include "logging.h"
#include "globals.h"
#include <stdlib.h>

disk *initDisk() {
    disk *new = (disk *)malloc(sizeof(disk));
    new->queue = initQueue();
    new->processing = 0;
}

void handleEventDisk(disk *d, event *e, heap *h) {
    switch(e->type) {
        case JOB_ARRIVAL:
            writeToLog("%d,%d,Arrived at disk %d\n", currentTime, e->pid, e->device);
            addProcessDisk(d, e, h);
            break;
        case JOB_COMPLETION:
            finishProcessDisk(d, h);
            break;
        default:
            break;
    }
}

void addProcessDisk(disk *d, event *e, heap *h) {
    pushQueue(d->queue, e);
    if (!d->processing) {
        startProcessDisk(d, h);
    }
}

void startProcessDisk(disk *d, heap *h) {
    // Set the Disk state to reflect the running job
    event *current = peekQueue(d->queue);
    current->startTime = currentTime;
    if (current->device == DISK1) {
        logResponseTime(currentTime - current->time, DISK1);
    } else {
        logResponseTime(currentTime - current->time, DISK2);
    }
    d->processing = 1;
    writeToLog("%d,%d,Job started at disk %d.\n", currentTime, current->pid, current->device);
    // Add the job end time to the priority queue
    int min = current->device == DISK1 ? DISK1_MIN : DISK2_MIN;
    int max = current->device == DISK1 ? DISK1_MAX : DISK2_MAX;
    event *end = initEvent();
    end->type = JOB_COMPLETION;
    end->device = current->device;
    end->time = rand() % (max + 1 - min) + min + currentTime;
    end->pid = current->pid;
    pushHeap(h, end);
}

void finishProcessDisk(disk *d, heap *h) {
    // Remove the process from the queue
    event *finished = popQueue(d->queue);
    logProcessingTime(currentTime - finished->startTime, finished->device);
    writeToLog("%d,%d,Job finished at disk %d.\n", currentTime, finished->pid, finished->device);
    if (finished->device == DISK1) {
        finishedJobsDisk1++;
    } else {
        finishedJobsDisk2++;
    }
    // Decide if the job should move to the disk
    writeToLog("%d,%d,Returning to CPU\n", currentTime, finished->pid);
    // Generate disk job
    event *toCPU = initEvent();
    toCPU->time = currentTime;
    toCPU->device = CPU;
    toCPU->pid = finished->pid;
    toCPU->type = JOB_ARRIVAL;
    // Add to priority queue
    pushHeap(h, toCPU);
    // Free old job
    free(finished);
    // Check if there is a job to start work on
    if (d->queue->length) {
        // There is, so start it
        startProcessDisk(d, h);
    } else {
        // There isn't, so set the CPU to idle
        d->processing = 0;
    }
}

disk *findShortestQueue(disk *disk1, disk *disk2) {
    return disk1->queue->length < disk2->queue->length ? disk1 : disk2;
}