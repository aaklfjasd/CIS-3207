#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "cpu.h"
#include "disk.h"
#include "logging.h"
#include "globals.h"

void handleEvent(event *);
void loadConfig();

heap *priorityQueue;
cpu *cpu1;
disk *disk1;
disk *disk2;

int main() {
    // Load config file
    loadConfig();

    logfile = fopen("log.csv", "w");

    processingTimeCPU = 0;
    responseTimeCPU = 0;
    maxResponseTimeCPU = 0;

    processingTimeDisk1 = 0;
    responseTimeDisk1 = 0;
    maxResponseTimeDisk1 = 0;

    processingTimeDisk2 = 0;
    responseTimeDisk2 = 0;
    maxResponseTimeDisk2 = 0;

    // Set up devices
    priorityQueue = initHeap(2);
    cpu1 = initCPU();
    disk1 = initDisk();
    disk2 = initDisk();

    jobs = 0;
    finishedJobsCPU = 0;
    finishedJobsDisk1 = 0;
    finishedJobsDisk2 = 0;

    // Seed RNG
    srand((unsigned) time(NULL));

    // Set up simulation start and end events
    event simStart = {
            .type = JOB_ARRIVAL,
            .time = INIT_TIME,
            .pid = 0
    };
    event simEnd = {
            .type = END_SIM,
            .time = FIN_TIME,
            .pid = 0
    };
    pushHeap(priorityQueue, &simStart);
    pushHeap(priorityQueue, &simEnd);

    //log("%d, %d, %d, %d, %f, %d, %d, %d, %d, %d, %d", INIT_TIME, FIN_TIME, ARRIVE_MIN, ARRIVE_MAX, QUIT_PROB, CPU_MIN, CPU_MAX, DISK1_MIN, DISK1_MAX, DISK2_MIN, DISK2_MAX);

    // Main program loop
    writeToLog("Time,PID,Event\n");
    int simFinished = 0;
    while(!simFinished) {
        // Get next event from queue
        event *current = popHeap(priorityQueue);
        currentTime = current->time;
        handleEvent(current);
        logQueueSizes(cpu1->queue->length, disk1->queue->length, disk2->queue->length);
        if (current->type == END_SIM) {
            simFinished = 1;
        }
    }

    fclose(logfile);
    writeStats();
    return 0;
}

void handleEvent(event *e) {
    // Move the event to the appropriate device.
    disk *location;
    switch(e->device) {
        case CPU:
            handleEventCPU(cpu1, e, priorityQueue);
            break;
        case DISK1:
            if (e->type == JOB_ARRIVAL) {
                location = findShortestQueue(disk1, disk2);
                e->device = location == disk1 ? DISK1 : DISK2;
                handleEventDisk(location, e, priorityQueue);
                break;
            }
            handleEventDisk(disk1, e, priorityQueue);
            break;
        case DISK2:
            handleEventDisk(disk2, e, priorityQueue);
            break;
        default:
            break;
    }
}

void loadConfig() {
    FILE *config = fopen("config", "r");
    if (config) {
        char varname[11];
        char valueString[MAX_VAR_LEN];
        int numScanned = 2;
        numScanned = fscanf(config, "%s %s", varname, valueString);
        while (numScanned == 2) {
            if (strcmp(varname, "INIT_TIME") == 0) {
                INIT_TIME = atoi(valueString);
            } else if (strcmp(varname, "FIN_TIME") == 0) {
                FIN_TIME = atoi(valueString);
            } else if (strcmp(varname, "ARRIVE_MIN") == 0) {
                ARRIVE_MIN = atoi(valueString);
            } else if (strcmp(varname, "ARRIVE_MAX") == 0) {
                ARRIVE_MAX = atoi(valueString);
            } else if (strcmp(varname, "QUIT_PROB") == 0) {
                QUIT_PROB = atof(valueString);
            } else if (strcmp(varname, "CPU_MIN") == 0) {
                CPU_MIN = atoi(valueString);
            } else if (strcmp(varname, "CPU_MAX") == 0) {
                CPU_MAX = atoi(valueString);
            } else if (strcmp(varname, "DISK1_MIN") == 0) {
                DISK1_MIN = atoi(valueString);
            } else if (strcmp(varname, "DISK1_MAX") == 0) {
                DISK1_MAX = atoi(valueString);
            } else if (strcmp(varname, "DISK2_MIN") == 0) {
                DISK2_MIN = atoi(valueString);
            } else if (strcmp(varname, "DISK2_MAX") == 0) {
                DISK2_MAX = atoi(valueString);
            }
            numScanned = fscanf(config, "%s %s", varname, valueString);
        }
        if (numScanned != EOF) {
            printf("Config file could not be read");
            exit(1);
        }
        fclose(config);
    } else {
        printf("Could not load config file");
        exit(1);
    }
}