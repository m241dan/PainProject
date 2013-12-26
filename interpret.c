/*
 * This file handles command interpreting
 */
#include <sys/types.h>
#include <stdio.h>

/* include main header file */
#include "mud.h"

void handle_cmd_input(D_SOCKET *dsock, char *arg)
{
  D_MOBILE *dMob;
  char command[MAX_BUFFER];
  bool found_cmd = FALSE;
  int i;

  if ((dMob = dsock->player) == NULL)
    return;

  arg = one_arg(arg, command);

  for (i = 0; tabCmd[i].cmd_name[0] != '\0' && !found_cmd; i++)
  {
    if (tabCmd[i].level > dMob->level) continue;

    if (is_prefix(command, tabCmd[i].cmd_name))
    {
      found_cmd = TRUE;
      (*tabCmd[i].cmd_funct)(dMob, arg);
    }
  }

  if (!found_cmd)
    text_to_mobile(dMob, "No such command.\n\r");
}

/*
 * The command table, very simple, but easy to extend.
 */
const struct typCmd tabCmd [] =
{

 /* command          function        Req. Level   State               */
 /* ----------------------------------------------------------------- */

  { "commands",      cmd_commands,   LEVEL_GUEST, STATE_PLAYING },
  { "compress",      cmd_compress,   LEVEL_GUEST, STATE_PLAYING },
  { "copyover",      cmd_copyover,   LEVEL_GOD,   STATE_PLAYING },
  { "help",          cmd_help,       LEVEL_GUEST, STATE_PLAYING },
  { "linkdead",      cmd_linkdead,   LEVEL_ADMIN, STATE_PLAYING },
  { "say",           cmd_say,        LEVEL_GUEST, STATE_PLAYING },
  { "save",          cmd_save,       LEVEL_GUEST, STATE_PLAYING },
  { "shutdown",      cmd_shutdown,   LEVEL_GOD,   STATE_PLAYING },
  { "quit",          cmd_quit,       LEVEL_GUEST, STATE_PLAYING },
  { "who",           cmd_who,        LEVEL_GUEST, STATE_PLAYING },
  /* account commands */
  { "quit",          act_quit,       LEVEL_BASIC, STATE_ACCOUNT },

  /* end of table */
  { "", 0 }
};

