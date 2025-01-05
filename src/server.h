#ifndef _SERVER_H_
#define _SERVER_H_

#include <sys/stat.h>

#include "dequeue.h"

struct ServerResourceMonitor {
  const char *path;
  struct stat stat;
};

struct ServerSettings {
  int port;
  int thread_count;
  char *hostname;
  dequeue_t resources; // dequeue of struct ServerResouceMonitor
};

struct ServerSettings server_default_settings();
int server_run(struct ServerSettings *);


#endif
