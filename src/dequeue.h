#ifndef _DEQUEUE_H_
#define _DEQUEUE_H_

#include <stdlib.h>

typedef struct _node {
  struct _node *prev;
  struct _node *next;
  void *value;
} _dequeue_node;

typedef struct _dequeue {
  _dequeue_node *head;
  _dequeue_node *tail;
  size_t length;
} dequeue_t;

void dequeue_init(dequeue_t *);
void dequeue_append(dequeue_t *, void *, size_t);
void dequeue_free(dequeue_t *);

#endif
