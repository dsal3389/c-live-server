#ifndef _DEQUEUE_H_
#define _DEQUEUE_H_

#include <stdlib.h>

typedef struct _node {
  struct _node *prev;
  struct _node *next;
  void *value;
} dequeue_node_t;

typedef struct _dequeue {
  dequeue_node_t *head;
  dequeue_node_t *tail;
  size_t length;
} dequeue_t;

void dequeue_init(dequeue_t *);
void dequeue_append(dequeue_t *, void *, size_t);
void *dequeue_pop_left(dequeue_t *);
void dequeue_free(dequeue_t *);

#endif
