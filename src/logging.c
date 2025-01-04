#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "logging.h"

// TODO: make thread safe
struct LoggingSettings loggingSettings;

void logging_init() {
  memset(&loggingSettings.fds, -1, sizeof(loggingSettings.fds));
  loggingSettings.level = LOG_INFO;
  loggingSettings.fd_count = 0;
}

void logging_print(enum LoggingLevel level, const char *fmt, ...) {
  if (level < loggingSettings.level) {
    return;
  }

  va_list args;
  char buffer[LOGGING_MAX_LINE_BUFFER];
  memset(buffer, 0, sizeof(buffer));

  va_start(args, fmt);
  int buffer_size = vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  // TODO: make thread safe
  for (int i = 0; i < loggingSettings.fd_count; i++) {
    write(loggingSettings.fds[i], buffer, buffer_size);
  }
}

void logging_set_level(enum LoggingLevel level) {
  loggingSettings.level = level;
}

void logging_add_fd(int fd) {
  if (loggingSettings.fd_count >= LOGGING_MAX_FDS) {
    fatal("attempt to add more logging files then possible, max possible files "
          "are %d",
          LOGGING_MAX_FDS);
  }
  loggingSettings.fds[loggingSettings.fd_count] = fd;
  loggingSettings.fd_count++;
}
