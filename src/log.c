#include <stdio.h>
#include <errno.h>
#include "log.h"

int log_error(int error, const char *message) {
  fprintf(stderr, "%s: %s\n", message, strerror(-error));
  return error;
}