#include "cpu.h"
#include "logging.h"
#include "globals.h"
#include <stdlib.h>

// Function to return a new, empty CPU
cpu *initCPU() {
    cpu *new = (cpu *)malloc(sizeof(cpu *));
    new->queue = initQueue();
    new->processing = 0;
    new->currentPID = 0;
    return new;
}

// event handling logic
void handleEventCPU(cpu *c, event *e, heap *h) {
    switch(e->type) {
        case JOB_ARRIVAL:
            // Add the new job to the cpu
            addProcessCPU(c, e, h);
            break;
        case JOB_COMPLETION:
            // Tell the CPU to finish the process
            finishProcessCPU(c, h);
            break;
        default:
            break;
    }
}

// Add a process to the CPU queue
void addProcessCPU(cpu *c, event *e, heap *h) {
    writeToLog("%d,%d,Job arrived at CPU.\n", currentTime, e->pid);
    // Add the process to the queue
    pushQueue(c->queue, e);
    // If the cpu is not working, start the process
    if (!c->processing) {
        startProcessCPU(c, h);
    }
    // Add a new job to the priority queue
    if (e->pid == jobs) {
        event *new = initEvent();
        new->type = JOB_ARRIVAL;
        new->device = CPU;
        new->time = rand() % (ARRIVE_MAX + 1 - ARRIVE_MIN) + ARRIVE_MIN + e->time;
        new->pid = ++jobs;
        pushHeap(h, new);
    }
}

// Start a process
void startProcessCPU(cpu *c, heap *h) {
    // Set the CPU state to reflect the running job
    event *current = peekQueue(c->queue);
    current->startTime = currentTime;
    logResponseTime(currentTime - current->time, CPU);
    c->currentPID = current->pid;
    c->processing = 1;
    writeToLog("%d,%d,Job started at CPU.\n", currentTime, current->pid);
    // Add the job end time to the priority queue
    event *end = initEvent();
    end->type = JOB_COMPLETION;
    end->device = CPU;
    end->time = rand() % (CPU_MAX + 1 - CPU_MIN) + CPU_MIN + currentTime;
    end->pid = current->pid;
    pushHeap(h, end);
}

// Finish the process
void finishProcessCPU(cpu *c, heap *h) {
    // Remove the process from the queue
    event *finished = popQueue(c->queue);
    writeToLog("%d,%d,Job finished at CPU.\n", currentTime, finished->pid);
    logProcessingTime(currentTime - finished->startTime, CPU);
    finishedJobsCPU++;
    // Decide if the job should move to the disk
    if (rand() < QUIT_PROB * RAND_MAX) {
        writeToLog("%d,%d,Job complete.\n", currentTime, finished->pid);
    } else {
        writeToLog("%d,%d,Going to disk\n", currentTime, finished->pid);
        // Generate disk job
        event *toDisk = initEvent();
        toDisk->time = currentTime;
        toDisk->device = DISK1;
        toDisk->pid = finished->pid;
        toDisk->type = JOB_ARRIVAL;
        // Add to priority queue
        pushHeap(h, toDisk);
    }
    // Free the finished event
    free(finished);
    // Check if there is a job to start work on
    if (c->queue->length) {
        // There is, so start it
        startProcessCPU(c, h);
    } else {
        // There isn't, so set the CPU to idle
        c->processing = 0;
        c->currentPID = 0;
    }
}