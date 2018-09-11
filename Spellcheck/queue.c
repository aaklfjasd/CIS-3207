//
// Created by angry on 3/14/2018.
//

#include <stdlib.h>
#include "queue.h"

queue *initQueue() {
    queue *new = (queue *)malloc(sizeof(queue));
    new->head = NULL;
    new->tail = NULL;
    new->len = 0;
    return new;
}

node *initNode(void *data) {
    node *new = (node *)malloc(sizeof(node));
    new->next = NULL;
    new->data = data;
    return new;
}

void addQueue(queue *queue, void *data) {
    node *new = initNode(data);
    if (queue->len == 0) {
        queue->head = new;
        queue->tail = new;
    } else {
        queue->tail->next = new;
        queue->tail = new;
    }
    queue->len += 1;
}

void *removeQueue(queue *queue) {
    if (queue->len == 0) {
        return NULL;
    }
    node *pop = queue->head;
    queue->head = pop->next;
    void *ret = pop->data;
    free(pop);
    queue->len -= 1;
    if (queue->len == 0) {
        queue->head = NULL;
        queue->tail = NULL;
    }
    return ret;
}

int empty(queue *q) {
    return q->len == 0;
}