#include <string.h>
#include <sys/stat.h>

#include "dequeue.h"
#include "logging.h"
#include "server.h"

__attribute__((always_inline)) inline struct ServerSettings server_default_settings() 
{
  struct ServerSettings default_settings;
  default_settings.thread_count = 5;
  default_settings.hostname = "127.0.0.1";
  default_settings.port = 8080;
  dequeue_init(&default_settings.resources);
  return default_settings;
}

// checks if given resource is valid
int server_resource_is_valid(struct ServerResourceMonitor *resource)
{
  const char *path = resource->path;

  // path cannot be empty or be absolute path
  if (*path == '\0' || *path == '/') {
    if (*path == '\0') {
      fatal("server: somehow got resource with path length 0, this is a bug");
    }

    log_error("server: resource cannot start with a `/`");
    return 0;
  }
  return 1;
}

void server_validate_resources(dequeue_t *resources) 
{
  if (resources->length == 0) {
    fatal("server: no resources to monitor were given");
  }

  dequeue_node_t *node = resources->head;
  int has_error = 0;

  while(node != NULL) {
    struct ServerResourceMonitor *resource = node->value;
    if(!server_resource_is_valid(resource)) {
      has_error = 1;
    }
    node = node->next;
  }

  if (has_error) {
    fatal("server: aborting due to invalid resources");
  }
}

void monitor_resources(dequeue_t *resources)
{
  dequeue_node_t *node = resources->head;
  while(node != NULL) {
    struct ServerResourceMonitor *resource = (struct ServerResourceMonitor *) node->value;
    log_info("resource path is %s", resource->path);
    node = node->next;
  }
}


int server_run(struct ServerSettings *settings) 
{
  server_validate_resources(&(settings->resources));
  monitor_resources(&(settings->resources));
  log_info("server started at %s:%d", settings->hostname, settings->port);
  return 0;
}
