#include <stdlib.h>
#include <string.h>

#include "dequeue.h"

void dequeue_init(dequeue_t *q) {
  q->length = 0;
  q->head = NULL;
  q->tail = NULL;
}

void dequeue_append(dequeue_t *q, void *item, size_t size) {
  _dequeue_node *n = malloc(sizeof(_dequeue_node));
  unsigned char *buffer = malloc(size);
  memcpy(buffer, item, size);

  n->value = buffer;
  n->prev = q->tail;
  n->next = NULL;

  if (q->head == NULL) {
    q->head = n;
  } else {
    q->tail->next = n;
  }
  q->tail = n;
}

void dequeue_free(dequeue_t *q) {
  _dequeue_node *n = q->head;

  while (n != NULL) {
    _dequeue_node *next = n->next;
    free(n->value);
    free(n);
    n = next;
  }

  // reset the queue state
  dequeue_init(q);
}
