#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <wrc.h>

#include "shell.h"

static int cmd_time(const char *args[])
{
  /* Stub */
  return 0;
}

DEFINE_WRC_COMMAND(time) = {
  .name = "time",
  .exec = cmd_time,
};
