#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "logging.h"

struct LoggingSettings loggingSettings = LOGGING_DEFAULT_INIT;

void logging_init() 
{
  memset(&loggingSettings.fds, -1, sizeof(loggingSettings.fds));
  loggingSettings.level = LOG_INFO;
  loggingSettings.fd_count = 0;
}

void logging_print(enum LoggingLevel level, const char *fmt, ...) 
{
  if (level < loggingSettings.level) {
    return;
  }

  va_list args;
  char buffer[LOGGING_MAX_LINE_BUFFER];
  memset(buffer, 0, sizeof(buffer));

  va_start(args, fmt);
  int buffer_size = vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  static pthread_mutex_t write_lock = PTHREAD_MUTEX_INITIALIZER;

  pthread_mutex_lock(&write_lock);
  for (int i = 0; i < loggingSettings.fd_count; i++) {
    write(loggingSettings.fds[i], buffer, buffer_size);
  }
  pthread_mutex_unlock(&write_lock);
}

// this function is should not be called from a thread that
// is not the main thread
void logging_set_level(enum LoggingLevel level) 
{
  loggingSettings.level = level;
}

// this function is should not be called from a thread that
// is not the main thread
void logging_add_fd(int fd) 
{
  if (loggingSettings.fd_count >= LOGGING_MAX_FDS) {
    fatal("attempt to add more logging files then possible, max possible files "
          "are %d",
          LOGGING_MAX_FDS);
  }
  loggingSettings.fds[loggingSettings.fd_count] = fd;
  loggingSettings.fd_count++;
}
