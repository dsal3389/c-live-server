#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "dequeue.h"
#include "logging.h"
#include "server.h"

// returns a boolean value indicating if the given ascii string
// contain all valid numbers and is safe to convert to int
int ascii_number(const char *s) {
  const char *c = s;
  while (*c) {
    if (!isdigit(*c++)) {
      return 0;
    }
  }
  return 1;
}

void help_message() { fatal("TODO: write help message\n"); }

// a simple macro to move to the next flag, and check that there is a next flag
// if there are no more arguments, we exit with given error message
#define parse_argv_require_flag_value(message)                                 \
  do {                                                                         \
    (*argc)--;                                                                 \
    (*argv)++;                                                                 \
    if (*argc == 0)                                                            \
      fatal(message);                                                          \
  } while (0)

void parse_argv(int *argc, char ***argv, struct ServerSettings *settings) {
  while (*argc) {
    if (!strcmp(**argv, "-h") || !strcmp(**argv, "--help")) {
      help_message();
    } else if (!strcmp(**argv, "-h") || !strcmp(**argv, "--hostname")) {
      parse_argv_require_flag_value(
          "args: --hostname requires hostname value as next argument");
      settings->hostname = **argv;
    } else if (!strcmp(**argv, "-p") || !strcmp(**argv, "--port")) {
      parse_argv_require_flag_value(
          "args: --port requires port value as next argument");

      if (!ascii_number(**argv)) {
        fatal("args: given port `%s` doesn't contain valid numbers", **argv);
      }
      settings->port = atoi(**argv);
    } else if (!strcmp(**argv, "-v") || !strcmp(**argv, "--debug")) {
      logging_set_level(LOG_DEBUG);
    } else if (!strcmp(**argv, "-q") || !strcmp(**argv, "--quite")) {
      logging_set_level(LOG_DISABLE);
    } else if (!strcmp(**argv, "-w") || !strcmp(**argv, "--warning")) {
      logging_set_level(LOG_WARNING);
    } else if (!strcmp(**argv, "-o") || !strcmp(**argv, "--output-log")) {
      parse_argv_require_flag_value(
          "args: --output-log expected a path to a logfile");

      int fd = open(**argv, O_RDWR | O_CREAT | O_APPEND);
      if (fd == -1) {
        fatal("args: failed to open/create file at path %s", **argv);
      }
      logging_add_fd(fd);
    } else {
      struct ServerResourceMonitor ssm;
      if (stat(**argv, &ssm.stat) != 0) {
        fatal("args: couldn't find given path `%s`", **argv);
      }

      ssm.path = **argv;
      dequeue_append(&(settings->resources), &ssm,
                     sizeof(struct ServerResourceMonitor));
    }

    (*argc)--;
    (*argv)++;
  }
}

int main(int argc, char **argv) {
  struct ServerSettings settings = server_default_settings();
  argc--;
  argv++;

  logging_init();
  logging_add_fd(STDOUT_FILENO);
  parse_argv(&argc, &argv, &settings);
  return server_run(&settings);
}
