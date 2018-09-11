#include <stdlib.h>
#include "globals.h"

event *initEvent() {
    event *new = (event *)malloc(sizeof(event));
    new->device = -1;
    new->type = -1;
    new->pid = -1;
    new->time = -1;
    new->startTime = -1;
}

// Add an event to the heap
void pushHeap(heap *h, event *e){
    // Ensure the heap is large enough to hold all events
    if (h->length + 1 >= h->size) {
        // If the heap has size 0, then give it size 4. Else, double the size.
        h->size = h->size ? h->size * 2 : 4;
        // Reallocate the array to be the correct size.
        h->events = (event **)realloc(h->events, h->size * sizeof(event *));
    }
    // Begin heap insert algorithm
    int loc = h->length;
    int parent = (loc - 1) / 2;
    // Loop until the new event has a larger time than the parent event
    while (loc > 0 && h->events[parent]->time > e->time) {
        // If the parent time is larger, move the parent to the current location, and iterate again
        h->events[loc] = h->events[parent];
        loc = parent;
        parent = (parent - 1) / 2;
    }
    // Appropriate place in heap found, insert the event.
    h->events[loc] = e;
    h->length++;
}

event *popHeap(heap *h){
    // Return NULL if there are no elements in the heap
    if (!h->length) {
        return NULL;
    }
    // Save the first event, to be returned later.
    event *popped = h->events[0];
    // Move bottom of heap to top
    h->events[0] = h->events[h->length - 1];
    // Reorder heap
    int loc = 0;
    int swap = loc;
    while (1) {
        event *move = h->events[loc];
        // Check if parent is larger than left child
        if (loc * 2 + 1 < h->length && move->time > h->events[loc * 2 + 1]->time) {
            swap = loc * 2 + 1;
        }
        // Check if right child is smaller
        if (loc * 2 + 2 < h->length && h->events[swap]->time > h->events[loc * 2 + 2]->time) {
            swap = loc * 2 + 2;
        }
        // Check if parent is smaller than either child
        if (swap == loc) {
            break;
        }
        h->events[loc] = h->events[swap];
        h->events[swap] = move;
        loc = swap;
    }
    h->length--;
    return popped;
}

heap *initHeap(int size) {
    heap *h = (heap *)malloc(sizeof(heap));
    h->size = size;
    h->length = 0;
    h->events = (event **)malloc(size *sizeof(event *));
}

void pushQueue(queue *q, event *e) {
    queueNode *new = (queueNode *)malloc(sizeof(queueNode));
    new->this = e;
    new->next = NULL;
    if (q->length == 0) {
        q->head = new;
        q->tail = new;
        q->length++;
        return;
    }
    q->tail->next = new;
    q->tail = new;
    q->length++;
}

event *popQueue(queue *q) {
    if (q->length == 0) {
        return NULL;
    }
    event *popped = q->head->this;
    q->head = q->head->next;
    q->length--;
    if (q->length == 0) {
        q->tail = NULL;
    }
    return popped;
}

event *peekQueue(queue *q) {
    if (q->head == NULL) {
        return NULL;
    }
    return q->head->this;
}

queue *initQueue() {
    queue *q = (queue *)malloc(sizeof(queue));
    q->length = 0;
    q->head = NULL;
    return q;
}