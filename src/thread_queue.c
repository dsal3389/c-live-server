#include <stdlib.h>
#include <pthread.h>

#include "logging.h"
#include "dequeue.h"
#include "thread_queue.h"

void threadq_init(threadq_queue_t *tq)
{
  pthread_mutex_init(&tq->lock, NULL);
  pthread_cond_init(&tq->cond, NULL);
  dequeue_init(&tq->dequeue);
}

void *threadq_get(threadq_queue_t *tq, void *buffer, size_t size)
{
  if (pthread_mutex_lock(&tq->lock) != 0) {
    fatal_with_errno("thread-queue", "locking on `get` returned an error");
  }
  
  // if there are no items on the queue
  // then wait for some event
  while (tq->dequeue.length == 0) {
    pthread_cond_wait(&tq->cond, &tq->lock);
  }

  void *item = dequeue_pop_left_b(&tq->dequeue, buffer, size);
  if (pthread_mutex_unlock(&tq->lock) != 0) {
    fatal_with_errno("thread-queue", "unlocking on `get` returned an error");
  }
  return item;
}

void threadq_put(threadq_queue_t *tq, void *item, size_t size)
{
  if (pthread_mutex_lock(&tq->lock) != 0) {
    fatal_with_errno("thread-queue", "locking on `put` returned an error");
  }
  dequeue_append(&tq->dequeue, item, size);
  if (pthread_mutex_unlock(&tq->lock) != 0) {
    fatal_with_errno("thread-queue", "unlocking on `put` returned an error");
  }
  pthread_cond_signal(&tq->cond);
}

