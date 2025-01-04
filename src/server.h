#ifndef _SERVER_H_
#define _SERVER_H_

#include <sys/stat.h>

#include "dequeue.h"

struct ServerResourceMonitor {
  const char *path;
  struct stat stat;
};

struct ServerSettings {
  dequeue_t resources; // dequeue of struct ServerResouceMonitor
  char *hostname;
  int port;
};

struct ServerSettings server_default_settings();
int server_run(struct ServerSettings *);

#endif
