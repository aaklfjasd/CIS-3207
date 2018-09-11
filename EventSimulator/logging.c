#include <stdarg.h>
#include <stdio.h>
#include "globals.h"
#include "logging.h"

int queueSizeCalls = 0;

void writeToLog(char *pattern, ...) {
    va_list list;
    va_start(list, pattern);
    //vprintf(pattern, list);
    vfprintf(logfile, pattern, list);
    va_end(list);
}

void logProcessingTime(int time, int device) {
    switch(device) {
        case CPU:
            processingTimeCPU += time;
            break;
        case DISK1:
            processingTimeDisk1 += time;
            break;
        case DISK2:
            processingTimeDisk2 += time;
            break;
        default:
            break;
    }
}

void logResponseTime(int time, int device) {
    switch(device) {
        case CPU:
            responseTimeCPU += time;
            if (time > maxResponseTimeCPU) {
                maxResponseTimeCPU = time;
            }
            break;
        case DISK1:
            responseTimeDisk1 += time;
            if (time > maxResponseTimeDisk1) {
                maxResponseTimeDisk1 = time;
            }
            break;
        case DISK2:
            responseTimeDisk2 += time;
            if (time > maxResponseTimeDisk2) {
                maxResponseTimeDisk2 = time;
            }
            break;
        default:
            break;
    }
}

void logQueueSizes(int cpu, int disk1, int disk2) {
    queueSizeCalls++;
    averageQueueSizeCPU += cpu;
    averateQueueSizeDisk1 += disk1;
    averageQUeueSizeDisk2 += disk2;
    if (cpu > maxQueueSizeCPU) {
        maxQueueSizeCPU = cpu;
    }
    if (disk1 > maxQueueSizeDisk1) {
        maxQueueSizeDisk1 = disk1;
    }
    if (disk2 > maxQueueSizeDisk2) {
        maxQueueSizeDisk2 = disk2;
    }
}

void writeStats() {
    FILE *stats = fopen("stats.txt", "w");
    fprintf(stats, "Queue Sizes:\n");
    fprintf(stats, "CPU Average: %f CPU Max: %d Disk1 Average: %f Disk1 Max: %d Disk2 Average: %f Disk2 Max: %d\n", (double)averageQueueSizeCPU/queueSizeCalls, maxQueueSizeCPU, (double)averateQueueSizeDisk1/queueSizeCalls, maxQueueSizeDisk1, (double)averageQUeueSizeDisk2/queueSizeCalls, maxQueueSizeDisk2);
    fprintf(stats, "Response Time:\n");
    fprintf(stats, "CPU Average: %f CPU Max: %d Disk1 Average: %f Disk1 Max: %d Disk2 Average: %f Disk2 Max: %d\n", (double)responseTimeCPU/(finishedJobsCPU + 1), maxResponseTimeCPU, (double)responseTimeDisk1/(finishedJobsDisk1 + 1), maxResponseTimeDisk1, (double)responseTimeDisk2/(finishedJobsDisk2 + 1), maxResponseTimeDisk2);
    fprintf(stats, "Processing Time:\n");
    fprintf(stats, "CPU: %f Disk1: %f Disk2: %f\n", (double)processingTimeCPU/finishedJobsCPU, (double)processingTimeDisk1/finishedJobsDisk1, (double)processingTimeDisk2/finishedJobsDisk2);
    fprintf(stats, "Througput:\n");
    fprintf(stats, "CPU: %f Disk1: %f Disk2: %f\n", (double)finishedJobsCPU/FIN_TIME, (double)finishedJobsDisk1/FIN_TIME, (double)finishedJobsDisk2/FIN_TIME);
    fprintf(stats, "Utilization:\n");
    fprintf(stats, "CPU: %f Disk1: %f Disk2: %f\n", (double)processingTimeCPU/FIN_TIME, (double)processingTimeDisk1/FIN_TIME, (double)processingTimeDisk2/FIN_TIME);
    fclose(stats);
}