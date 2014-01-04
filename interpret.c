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

  { "commands",      cmd_commands,   LEVEL_PLAYER, STATE_PLAYING },
  { "compress",      cmd_compress,   LEVEL_PLAYER, STATE_PLAYING },
  { "copyover",      cmd_copyover,   LEVEL_GOD,   STATE_PLAYING },
  { "help",          cmd_help,       LEVEL_PLAYER, STATE_PLAYING },
  { "linkdead",      cmd_linkdead,   LEVEL_ADMIN, STATE_PLAYING },
  { "say",           cmd_say,        LEVEL_NPC, STATE_PLAYING },
  { "save",          cmd_save,       LEVEL_PLAYER, STATE_PLAYING },
  { "shutdown",      cmd_shutdown,   LEVEL_GOD,   STATE_PLAYING },
  { "quit",          cmd_quit,       LEVEL_PLAYER, STATE_PLAYING },
  { "who",           cmd_who,        LEVEL_PLAYER, STATE_PLAYING },
  /* account commands */
  { "quit",          act_quit,       LEVEL_BASIC, STATE_ACCOUNT },
  { "create",        act_create_char,LEVEL_BASIC, STATE_ACCOUNT },

  /* end of table */
  { "", 0 }
};

void new_handle_cmd_input(D_SOCKET *dsock, char *arg)
{
   ITERATOR Iter;
   LIST *cmd_table;
   COMMAND *com;
   void *entity;
   char command[MAX_BUFFER];
   bool found_cmd = FALSE;

  arg = one_arg(arg, command);

   switch( dsock->state )
   {
      case STATE_ACCOUNT:
         if( ( entity = dsock->account ) == NULL )
            return;
         cmd_table = dsock->account->commands;
         break;
      case STATE_PLAYING:
         if( ( entity = dsock->player ) == NULL )
            return;
         break;
   }

   AttachIterator( &Iter, cmd_table );
   while( ( com = (COMMAND *)NextInList(&Iter) ) != NULL )
      if( is_prefix( com->cmd_name, command ) )
      {
         (*com->cmd_funct)( entity, arg );
         found_cmd = TRUE;
         break;
      }
   DetachIterator(&Iter);

   if (!found_cmd)
     text_to_buffer(dsock, "No such command.\n\r");
   return;
}

