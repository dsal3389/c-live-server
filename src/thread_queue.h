#ifndef _THREAD_QUEUE_
#define _THREAD_QUEUE_

#include <pthread.h>

#include "dequeue.h"

typedef struct {
  dequeue_t dequeue;
  pthread_mutex_t lock;
  pthread_cond_t cond;
} threadq_queue_t;


void threadq_init(threadq_queue_t *);
void *threadq_get(threadq_queue_t *);
void threadq_put(threadq_queue_t *, void *, size_t);


#endif
