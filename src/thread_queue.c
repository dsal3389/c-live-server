#include <stdlib.h>
#include <pthread.h>

#include "dequeue.h"
#include "thread_queue.h"

void threadq_init(threadq_queue_t *tq)
{
  pthread_mutex_init(&tq->lock, NULL);
  pthread_cond_init(&tq->cond, NULL);
  dequeue_init(&tq->dequeue);
}

void *threadq_get(threadq_queue_t *tq)
{
  int s = pthread_mutex_lock(&tq->lock);
  
  // if there are no items on the queue
  // then wait for some event
  while (tq->dequeue.length == 0) {
    // TODO: timeout and put inside a while loop
    pthread_cond_wait(&tq->cond, &tq->lock);
  }

  void *item = dequeue_pop_left(&tq->dequeue);
  s = pthread_mutex_unlock(&tq->lock);
  return item;
}

void threadq_put(threadq_queue_t *tq, void *item, size_t size)
{
  int s = pthread_mutex_lock(&tq->lock);
  dequeue_append(&tq->dequeue, item, size);
  s = pthread_mutex_unlock(&tq->lock);
  pthread_cond_signal(&tq->cond);
}

