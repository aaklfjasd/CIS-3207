#ifndef GLOBALS
#define GLOBALS

#define MAX_VAR_LEN 200

#define JOB_ARRIVAL 0
#define JOB_START 1
#define JOB_COMPLETION 2
#define END_SIM 3

#define CPU 0
#define DISK1 1
#define DISK2 2

int INIT_TIME;
int FIN_TIME;
int ARRIVE_MIN;
int ARRIVE_MAX;
double QUIT_PROB;
int CPU_MIN;
int CPU_MAX;
int DISK1_MIN;
int DISK1_MAX;
int DISK2_MIN;
int DISK2_MAX;

int currentTime;
int jobs;
int finishedJobsCPU;
int finishedJobsDisk1;
int finishedJobsDisk2;

// Structure of an event to be added to the queue
typedef struct {
    int type;
    int time;
    int pid;
    int device;
    int startTime;
} event;

// Structure of the queue
typedef struct {
    event **events;
    int length;
    int size;
} heap;

typedef struct queueNode queueNode;

struct queueNode {
    event *this;
    queueNode *next;
};

typedef struct {
    queueNode *head;
    queueNode *tail;
    int length;
} queue;

event *initEvent();

void pushHeap(heap *, event *);
event *popHeap(heap *);
heap *initHeap(int);

void pushQueue(queue *, event *);
event *popQueue(queue *);
event *peekQueue(queue *);
queue *initQueue();

#endif