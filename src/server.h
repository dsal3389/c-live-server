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
  int worker_count;
  char *hostname;
  dequeue_t resources; // dequeue of struct ServerResouceMonitor
};

struct ServerSettings server_default_settings();

// start the dev server loop and resource monitoring
// with the given settings
void server_start(struct ServerSettings *);


#endif
