#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "ansi.h"

#define LOGGING_MAX_FDS 16
#define LOGGING_MAX_LINE_BUFFER 1024
#define LOGGING_DEFAULT_INIT { .level = LOG_INFO, .fd_count = 0, .fds = {} }

#define LOGGING_DEFAULT_FORMAT(level_, prefix_, details_) \
  " " BMAG level_ BWHT " | " BCYN prefix_ BWHT " - " details_ "\n"


// logging helper macros
#define log_debug(prefix, fmt, ...)                                                    \
  logging_print(LOG_DEBUG,   LOGGING_DEFAULT_FORMAT("DEBUG   ", prefix, fmt) __VA_OPT__(,) __VA_ARGS__)
#define log_info(prefix, fmt, ...)                                                     \
  logging_print(LOG_INFO,    LOGGING_DEFAULT_FORMAT("INFO    ", prefix, fmt) __VA_OPT__(,) __VA_ARGS__)
#define log_warning(prefix, fmt, ...)                                                  \
  logging_print(LOG_WARNING, LOGGING_DEFAULT_FORMAT("WARN    ", prefix, fmt) __VA_OPT__(,) __VA_ARGS__)
#define log_error(prefix, fmt, ...)                                                    \
  logging_print(LOG_ERROR,   LOGGING_DEFAULT_FORMAT("ERROR   ", prefix, fmt) __VA_OPT__(,) __VA_ARGS__)
#define log_critical(prefix, fmt, ...)                                                 \
  logging_print(LOG_DEBUG,   LOGGING_DEFAULT_FORMAT("CRITICAL", prefix, fmt) __VA_OPT__(,) __VA_ARGS__)

#define fatal(prefix, fmt, ...)                                                \
  do {                                                                         \
    log_critical(prefix, fmt, __VA_ARGS__);                                    \
    exit(1);                                                                   \
  } while (0)

#define fatal_with_errno(prefix, fmt, ...) \
  do {                                                                                            \
    log_critical(prefix, fmt "\nerrno: %s\n" __VA_OPT__(,) __VA_ARGS__, strerror(errno));         \
    exit(1);                                                                                      \
  } while (0)



enum LoggingLevel {
  LOG_DISABLE,
  LOG_DEBUG = 10,
  LOG_INFO = 20,
  LOG_WARNING = 30,
  LOG_ERROR = 40,
  LOG_CRITICAL = 50,
};

struct LoggingSettings {
  enum LoggingLevel level;

  // list of file descriptors that the logging output
  // will be written to, since 0 is a valid file descriptor, if the
  // field encounters -1, the logging should stop iterating over the list
  // so this list is `-1` terminating
  size_t fd_count;
  int fds[LOGGING_MAX_FDS];
};

extern struct LoggingSettings loggingSettings;

// add file descriptor for the logger to write to
void logging_add_fd(int);

// set the logging level
void logging_set_level(enum LoggingLevel);

// write log message in the given level
void logging_print(enum LoggingLevel, const char *, ...);

#endif
