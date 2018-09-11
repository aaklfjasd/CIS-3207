#ifndef LOGGING
#define LOGGING

#include <stdio.h>

int processingTimeCPU;
int processingTimeDisk1;
int processingTimeDisk2;

int responseTimeCPU;
int responseTimeDisk1;
int responseTimeDisk2;

int maxResponseTimeCPU;
int maxResponseTimeDisk1;
int maxResponseTimeDisk2;

int averageQueueSizeCPU;
int averateQueueSizeDisk1;
int averageQUeueSizeDisk2;

int maxQueueSizeCPU;
int maxQueueSizeDisk1;
int maxQueueSizeDisk2;

void writeToLog(char *, ...);
void logProcessingTime(int, int);
void logResponseTime(int, int);
void logQueueSizes(int, int, int);
void writeStats();

FILE *logfile;

#endif