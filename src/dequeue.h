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

// initialize the given dequeue element
void dequeue_init(dequeue_t *);

// appends item to the end of the queue, the item itself
// is copied from given buffer to a memory allocated buffer
void dequeue_append(dequeue_t *, void *, size_t);

// pop items from the end of the queue, the returned item 
// points to heap allocated memory that should be freed, returns 
// NULL if queue is empty and no item poped
void *dequeue_pop_left(dequeue_t *);

// pop item from the end of the queue into the given buffer, the poped item
// is written to the given buffer and freed from the heap, the function returns
// a pointer to the given buffer or NULL if there are no items to pop
void *dequeue_pop_left_b(dequeue_t *, void *, size_t);

// free all allocated memory by the queue
void dequeue_free(dequeue_t *);

#endif
