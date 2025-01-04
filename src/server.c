#include <sys/stat.h>

#include "dequeue.h"
#include "logging.h"
#include "server.h"

struct ServerSettings server_default_settings() {
  struct ServerSettings default_settings;
  default_settings.hostname = "127.0.0.1";
  default_settings.port = 8080;
  dequeue_init(&default_settings.resources);
  return default_settings;
}

int server_run(struct ServerSettings *settings) {
  log_info("server started at %s:%d", settings->hostname, settings->port);
  return 0;
}
