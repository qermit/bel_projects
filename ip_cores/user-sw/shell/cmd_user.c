#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <wrc.h>
#include "shell.h"

__attribute__((weak)) int user_main(void);
__attribute__((weak)) int user_main(void)
{
  return(0);
}

static int cmd_user(const char *args[])
{

  /* Show message */
  mprintf("Calling user function...\n");
  
  /* Call user function */
  user_main();

  /* Done */
  return(0);
  
}

DEFINE_WRC_COMMAND(user) = {
  .name = "user",
  .exec = cmd_user,
};
