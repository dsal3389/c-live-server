#ifndef _THREAD_QUEUE_
#define _THREAD_QUEUE_

#include <stdlib.h>
#include <pthread.h>

#include "dequeue.h"

typedef struct {
  dequeue_t dequeue;
  pthread_mutex_t lock;
  pthread_cond_t cond;
} threadq_queue_t;


// initialize the thread queue struct
void threadq_init(threadq_queue_t *);

// get item from the underlying queue, block until item is recved 
// the item will be pushed to the given buffer
void *threadq_get(threadq_queue_t *, void *, size_t);

// push item to the end of the queue, the given item will be copied 
// to a buffer on the heap
void threadq_put(threadq_queue_t *, void *, size_t);


#endif
