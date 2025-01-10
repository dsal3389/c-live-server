#include <stdlib.h>
#include <string.h>

#include "dequeue.h"

void dequeue_init(dequeue_t *q) 
{
  q->length = 0;
  q->head = NULL;
  q->tail = NULL;
}

void dequeue_append(dequeue_t *q, void *item, size_t size) 
{
  dequeue_node_t *n = malloc(sizeof(dequeue_node_t));
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
  q->length++;
}

// returned itme should be freed and check
// if returned item pointer is not NULL
void *dequeue_pop_left(dequeue_t *q)
{
  if (q->head == NULL) {
    return NULL;
  }

  dequeue_node_t *node = q->head;
  q->head = node->next;
  q->length--;

  if (q->tail == node) {
    q->tail = NULL;
  }

  void *value = node->value;
  free(node);
  return value;
}

void *dequeue_pop_left_b(dequeue_t *q, void *buffer, size_t size) {
  void *item = dequeue_pop_left(q);
  if(item == NULL) {
    return NULL;
  }

  memcpy(buffer, item, size);
  free(item);
  return buffer;
}

void dequeue_free(dequeue_t *q) 
{
  dequeue_node_t *n = q->head;

  while (n != NULL) {
    dequeue_node_t *next = n->next;
    free(n->value);
    free(n);
    n = next;
  }

  // reset the queue state
  dequeue_init(q);
}
