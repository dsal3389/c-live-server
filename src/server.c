#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>

#include "thread_queue.h"
#include "dequeue.h"
#include "logging.h"
#include "server.h"

struct WorkerArgs {
  int id;
  threadq_queue_t *tq;
};

// defines worker connkection item data
// this is used when there is new connection for the worker to handle
struct WorkerConnection {
  int fd;
  struct sockaddr_in addr;
  socklen_t addr_len;
};

// union that holds all variants 
// of the worker events
union WorkerData {
  struct WorkerConnection conn;
};

enum WorkerItemVariant {
  WORKER_CONNECTION_ITEM,
  WOKRER_FILE_ITEM
};

// the queued item that is pushed to the queue,
// the worker first verify what is the `variant`
// and based on that access the correct field in the enum
struct WorkerQueuedItem {
  enum WorkerItemVariant variant;
  union WorkerData data;
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

__attribute__((always_inline)) int server_bind_and_listen(const char *hostname, int port)
{
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    fatal_with_errno("server", "couldn't create socket");
  }
  if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1) {
    fatal_with_errno("server", "couldn't set nonblocking flag on socket fd");
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  // server_addr.sin_addr.s_addr = inet_addr(hostname);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(port);

  if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
    fatal_with_errno("server", "couldn't bind address %s:%d to socket.", hostname, port);
  }
  if (listen(sockfd, SERVER_MAX_CONNECTIONS) != 0) {
    fatal_with_errno("server", "couldn't listen on socket.");
  }
  return sockfd;
}

// checks if given resource is valid
int server_resource_is_valid(struct ServerResource*resource)
{
  const char *path = resource->path;

  // path cannot be empty or be absolute path
  if (*path == '\0' || *path == '/') {
    if (*path == '\0') {
      fatal("server", "somehow got resource with path length 0, this is a bug");
    }

    log_error("server", "resource cannot start with a `/`");
    return 0;
  }
  return 1;
}

void server_validate_resources(dequeue_t *resources) 
{
  if (resources->length == 0) {
    fatal("server", "no resources to monitor were given");
  }

  dequeue_node_t *node = resources->head;
  int has_error = 0;

  while(node != NULL) {
    struct ServerResource *resource = node->value;
    if(!server_resource_is_valid(resource)) {
      has_error = 1;
    }
    node = node->next;
  }

  if (has_error) {
    fatal("server", "aborting due to invalid resources");
  }
}

// preper the given pollfds with the server resources, it is expected
// that the given buffer to pollfd will have enough space for each resource
void server_preper_for_poll(struct pollfd *fds, dequeue_t *resources) {
  dequeue_node_t *node = resources->head;
  nfds_t nfds = 0;

  while(node != NULL) {
    struct ServerResource *resource = node->value;

    if((fds[nfds].fd = open(resource->path, O_RDONLY)) == -1) {
      fatal_with_errno("server", "couldn't open resource for monitoring at path `%s`", resource->path);
    }
    fds[nfds].events = POLLIN;

    nfds++;
    node = node->next;
  }
}

void server_handle_socket_event(int sockfd, threadq_queue_t *queue)
{
  struct WorkerQueuedItem item;
  struct WorkerConnection *conn = &item.data.conn;

  item.variant = WORKER_CONNECTION_ITEM;
  if((conn->fd = accept(sockfd, (struct sockaddr *) &conn->addr, &conn->addr_len)) == -1) {
    fatal_with_errno("server", "accept");
  }
  threadq_put(queue, &item, sizeof(struct WorkerQueuedItem));
}

void *worker_loop(void *args)
{
  struct WorkerArgs *wa = args;
  struct WorkerQueuedItem item_buffer;

  log_debug("server", "worker thread %d started", wa->id);

  for(;;) {
    struct WorkerQueuedItem *item = threadq_get(wa->tq, &item_buffer, sizeof(item_buffer));

    if (item->variant == WORKER_CONNECTION_ITEM) {
      log_info("server", "thread %d is handling a connection, socket fd %d", wa->id, item->data.conn.fd);
    } else {
      log_error("server", "unexpecte variant recved");
    }
  }
  return NULL;
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
      fatal_with_errno("server", "couldn't create worker thread number %d", i + 1);
    }
  }
}

void server_loop(struct ServerSettings *settings, threadq_queue_t *queue) 
{
  int poll_ready = 0;

  // +1 for the socket fd
  nfds_t nfds = settings->resources.length + 1;
  struct pollfd fds[nfds];

  // first pollfd is the server socket, always
  fds[0].events = POLLIN; //  | POLLHUP;
  fds[0].fd = server_bind_and_listen(settings->hostname, settings->port);

  log_info("server", "running http://%s:%d", settings->hostname, settings->port);
  // server_preper_for_poll(fds + 1, &settings->resources);

  for (;;) {
    if (poll(fds, 1, -1) == -1) {
      fatal_with_errno("server", "poll returned error");
    }

    log_debug("server", "event poll recv");

    if (fds[0].revents & POLLIN) {
      server_handle_socket_event(fds[0].fd, queue);
    }
  }
}

void server_start(struct ServerSettings *settings) 
{
  pthread_t workers[settings->worker_count];
  threadq_queue_t queue;
  threadq_init(&queue);

  server_validate_resources(&(settings->resources));
  server_start_workers(workers, settings->worker_count, &queue);
  server_loop(settings, &queue);
}
