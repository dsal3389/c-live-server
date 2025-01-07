#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <stdlib.h>

#define LOGGING_MAX_FDS 16
#define LOGGING_MAX_LINE_BUFFER 1024

// logging helper macros
#define log_debug(fmt, ...)                                                    \
  logging_print(LOG_DEBUG, fmt "\n" __VA_OPT__(,) __VA_ARGS__)
#define log_info(fmt, ...)                                                     \
  logging_print(LOG_INFO, fmt "\n" __VA_OPT__(,) __VA_ARGS__)
#define log_warning(fmt, ...)                                                  \
  logging_print(LOG_WARNING, fmt "\n" __VA_OPT__(,) __VA_ARGS__)
#define log_error(fmt, ...)                                                    \
  logging_print(LOG_ERROR, fmt "\n" __VA_OPT__(,) __VA_ARGS__)
#define log_critical(fmt, ...)                                                 \
  logging_print(LOG_CRITICAL, fmt "\n" __VA_OPT__(,) __VA_ARGS__)

#define fatal(fmt, ...)                                                        \
  do {                                                                         \
    logging_print(LOG_CRITICAL, fmt "\n" __VA_OPT__(,) __VA_ARGS__);          \
    exit(1);                                                                   \
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

void logging_init();
void logging_add_fd(int);
void logging_set_level(enum LoggingLevel);
void logging_print(enum LoggingLevel, const char *, ...);

#endif
