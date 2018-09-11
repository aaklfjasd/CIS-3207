#ifndef SPELLCHECK_QUEUE_H
#define SPELLCHECK_QUEUE_H

typedef struct node {
    void *data;
    struct node *next;
} node;

typedef struct {
    node *head;
    node *tail;
    int len;
} queue;

queue *initQueue();
node *initNode(void *);
void addQueue(queue *, void *);
void *removeQueue(queue *);
int empty(queue *);

#endif //SPELLCHECK_QUEUE_H
