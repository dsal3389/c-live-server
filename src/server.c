#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "thread_queue.h"
#include "dequeue.h"
#include "logging.h"
#include "server.h"

struct WorkerArgs {
  int id;
  threadq_queue_t *tq;
};

struct WorkerQueuedConnection {
  int fd;
  struct sockaddr_in addr;
  socklen_t addr_len;
};

__attribute__((always_inline)) inline struct ServerSettings server_default_settings() 
{
  struct ServerSettings default_settings;
  default_settings.worker_count = 5;
  default_settings.hostname = "127.0.0.1";
  default_settings.port = 8080;
  dequeue_init(&default_settings.resources);
  return default_settings;
}

int server_create_socket(const char *hostname, int port)
{
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd == -1) {
    fatal("server: couldn't create socket");
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(hostname);
  server_addr.sin_port = htons(port);

  if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    fatal("server: couldn't bind address %s:%d to socket", hostname, port);
  }
  return sockfd;
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

void *worker_loop(void *args)
{
  struct WorkerArgs *wa = args;
  log_debug("worker thread `%d` started", wa->id);

  for(;;) {
    struct WorkerQueuedConnection *connection = threadq_get(wa->tq);
    log_info("thread %d is handling a connection", wa->id);
  }
}

// create `n` number of threads, each thread listens for the queue 
// and wait for a request to process
void server_start_workers(pthread_t *worker_buff, int count, threadq_queue_t *queue)
{
  // this should be safe to live as long as the program lives because
  // it is used by the workes for the whole program lifetime
  struct WorkerArgs *args = malloc(sizeof(struct WorkerArgs) * count);

  for(int i = 0; i < count; i++) {
    struct WorkerArgs *wa = &(args[i]);
    wa->id = i + 1;
    wa->tq = queue;

    if(pthread_create(&(worker_buff[i]), NULL, worker_loop, wa) != 0) {
      fatal("server: couldn't create worker thread number %d", i + 1);
    }
  }
}

void server_loop(struct ServerSettings *settings, threadq_queue_t *requests_queue) 
{
  int sockfd = server_create_socket(settings->hostname, settings->port);

  struct WorkerQueuedConnection connection;
  connection.fd = 0;
  connection.addr_len = sizeof(connection.addr);

  if (listen(sockfd, 1024) != 0) {
    fatal("server: couldn't listen on socket, socket might already be in use.");
  }

  log_info("server running http://%s:%d", settings->hostname, settings->port);

  for(;;) {
    if((connection.fd = accept(sockfd, (struct sockaddr *) &connection.addr, &connection.addr_len)) == -1) {
      continue;
    }

    // the item will be copied to the queue 
    // so we can just reuse the same variable
    threadq_put(requests_queue, &connection, sizeof(struct WorkerQueuedConnection));
  }
}

void server_start(struct ServerSettings *settings) 
{
  pthread_t workers[settings->worker_count];
  threadq_queue_t requests_queue;
  threadq_init(&requests_queue);

  server_validate_resources(&(settings->resources));
  server_start_workers(workers, settings->worker_count, &requests_queue);
  server_loop(settings, &requests_queue);
}
