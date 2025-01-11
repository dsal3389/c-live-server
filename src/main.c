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
    if (!isdigit(*c)) {
      return 0;
    }
    c++;
  }
  return 1;
}

void help_message() { fatal("main", "TODO: write help message\n"); }

// a simple macro to move to the next flag, and check that there is a next flag
// if there are no more arguments, we exit with given error message
#define parse_argv_require_flag_value(message)                                 \
  do {                                                                         \
    (*argc)--;                                                                 \
    (*argv)++;                                                                 \
    if (*argc == 0)                                                            \
      fatal("args", message);                                                          \
  } while (0)

void parse_argv(int *argc, char ***argv, struct ServerSettings *settings) 
{
  while (*argc) {
    if (!strcmp(**argv, "-h") || !strcmp(**argv, "--help")) {
      help_message();
    } else if (!strcmp(**argv, "-h") || !strcmp(**argv, "--hostname")) {
      parse_argv_require_flag_value(
          "--hostname requires hostname value as next argument");
      settings->hostname = **argv;
    } else if (!strcmp(**argv, "-p") || !strcmp(**argv, "--port")) {
      parse_argv_require_flag_value(
          "--port requires port value as next argument");

      if (!ascii_number(**argv)) {
        fatal("args", "given port `%s` is not a valid number", **argv);
      }
      settings->port = atoi(**argv);
    } else if (!strcmp(**argv, "-w") || !strcmp(**argv, "--workers")) {
      parse_argv_require_flag_value("--workers expected a number");

      if(!ascii_number(**argv)) {
        fatal("args", "given worker `%s` is not a valid number", **argv);
      }
      settings->worker_count = atoi(**argv);
    } else if (!strcmp(**argv, "-v") || !strcmp(**argv, "--debug")) {
      logging_set_level(LOG_DEBUG);
    } else if (!strcmp(**argv, "-q") || !strcmp(**argv, "--quite")) {
      logging_set_level(LOG_DISABLE);
    } else if (!strcmp(**argv, "-w") || !strcmp(**argv, "--warning")) {
      logging_set_level(LOG_WARNING);
    } else if (!strcmp(**argv, "-o") || !strcmp(**argv, "--output-log")) {
      parse_argv_require_flag_value(
          "--output-log expected a path to a logfile");

      int fd = open(**argv, O_RDWR | O_CREAT | O_APPEND);
      if (fd == -1) {
        fatal_with_errno("args", "failed to open/create file at path %s", **argv);
      }
      logging_add_fd(fd);
    } else {
      if (***argv == '-') {
        fatal("args", "unknown flag `%s`", **argv);
      }

      struct ServerResource sr;
      if (stat(**argv, &sr.stat) != 0) {
        fatal("args", "couldn't find given path `%s`", **argv);
      }

      sr.path = **argv;
      dequeue_append(&(settings->resources), &sr,
                     sizeof(struct ServerResource));
    }

    (*argc)--;
    (*argv)++;
  }
}

int main(int argc, char **argv) 
{
  struct ServerSettings settings = server_default_settings();
  argc--;
  argv++;

  // add the standard output to the
  // logging destination
  logging_add_fd(STDOUT_FILENO);

  parse_argv(&argc, &argv, &settings);
  server_start(&settings);
  return 0;
}
