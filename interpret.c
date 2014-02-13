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
  { "look",          cmd_look,       LEVEL_PLAYER, STATE_PLAYING },
  { "north",         cmd_north,      LEVEL_NPC, STATE_PLAYING },
  { "east",          cmd_east,       LEVEL_NPC, STATE_PLAYING },
  { "south",         cmd_south,      LEVEL_NPC, STATE_PLAYING },
  { "west",          cmd_west,       LEVEL_NPC, STATE_PLAYING },
  { "up",            cmd_up,         LEVEL_NPC, STATE_PLAYING },
  { "down",          cmd_down,       LEVEL_NPC, STATE_PLAYING },
  { "w_open",        cmd_open_workspace, LEVEL_ADMIN, STATE_PLAYING },
  { "w_close",       cmd_close_workspace, LEVEL_ADMIN, STATE_PLAYING },
  { "pagewidth",     cmd_pagewidth,  LEVEL_PLAYER, STATE_PLAYING },
  /* account commands */
  { "settings",      act_settings,   LEVEL_BASIC, STATE_ACCOUNT },
  { "quit",          act_quit,       LEVEL_BASIC, STATE_ACCOUNT },
  { "create",        act_create_char,LEVEL_BASIC, STATE_ACCOUNT },

  /* end of table */
  { "", 0 }
};

const struct typCmd actSettingCmd[] =
{
   { "pagewidth", act_pagewidth, LEVEL_BASIC, STATE_ACCOUNT },

   /* end of table */
   { "", 0 }
};

/*
const struct typCmd olcCmd[] =
{
   { "list", cmd_list, LEVEL_ADMIN, STATE_PLAYING },
   { "create", cmd_create_framework, LEVEL_ADMIN, STATE_PLAYING },
   { "edit", cmd_edit_framework, LEVEL_ADMIN, STATE_PLAYING },
   { "", 0 }
}
*/
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
         cmd_table = dsock->player->commands;
         break;
   }

   AttachIterator( &Iter, cmd_table );
   while( ( com = (COMMAND *)NextInList(&Iter) ) != NULL )
   {
      if( is_prefix( command, com->cmd_name ) )
      {
         (*com->cmd_funct)( entity, arg );
         found_cmd = TRUE;
         break;
      }
   }
   DetachIterator(&Iter);

   if( !found_cmd && dsock->state == STATE_ACCOUNT && SizeOfList( dsock->account->characters ) >= 1 )
   {
      CHAR_SHEET *character;
      D_MOBILE *player;
      NANNY *nanny;
      AttachIterator( &Iter, dsock->account->characters );
      while( ( character = (CHAR_SHEET *)NextInList( &Iter ) ) != NULL )
         if( !strcasecmp( command, character->name ) )
         {
            found_cmd = TRUE;
            player = init_mobile();
            if( !load_mobile( get_loc_from_char_sheet( character ), player ) )
            {
               text_to_buffer( dsock, "The pFile could not be loaded.\r\n" );
               bug( "%s: could not load the mobile %s.", __FUNCTION__, character->name );
               free_mobile( player );
               return;
            }
            if( player->password && player->password[0] != '\0' )
            {
               change_socket_state( dsock, STATE_NANNY );
               nanny = init_nanny( NANNY_CHAR_PASS_CHECK );
               control_nanny( dsock, nanny );
               set_nanny_creation( nanny, player );
               change_nanny_state( dsock->nanny, NANNY_CHAR_PASS_CHECK_CONFIRM, TRUE );
            }
            else
            {
               control_player( dsock, player );
               char_to_game( player );
               change_socket_state( dsock, STATE_PLAYING );
            }
         }
      DetachIterator( &Iter );
   }


   if (!found_cmd)
     text_to_buffer(dsock, "No such command.\n\r");
   return;
}

CMD_FLAG *create_flag( char *flag )
{
   CMD_FLAG *cFlag;
   CREATE( cFlag, CMD_FLAG, 1 );
   cFlag->flag = strdup( flag );
   return cFlag;
}
/*
void pull_flags( LIST *flags, char *arg )
{
   CMD_FLAG *cFlag;
   char analyzed[MAX_BUFFER];

   while( arg && arg[0] != '\0' )
   {
      arg = one_arg( arg, analyzed );

      if( analyzed[0] == '-' )
      {
         cFlag = create_flag( analyzed );
         AttachToList( cFlag, flags );
      }

   }

   return;
}
*/
void pull_flags( LIST *flags, char *arg, char *arg_no_flags )
{
   CMD_FLAG *cFlag;
   char parambuf[MAX_BUFFER];
   char flagbuf[MAX_BUFFER];
   char *ptr;
   int mode = 1; /* 1 arg_no_flags, 2 parambuf, 3 flagbuf */

   ptr = arg_no_flags;

   /* remove leading spaces */
   while( isspace( *arg ) )
      arg++;

   while( *arg != '\0' )
   {
      if( *arg == '-' && mode == 1 )
      {
         mode = 3;
         ptr = flagbuf;
      }
      else if( *arg == '(' && mode == 3 )
      {
         arg++;
         mode = 2;
         ptr = parambuf;
         cFlag = create_flag( flagbuf );
         AttachToList( cFlag, flags );
         memset( &flagbuf[0], 0, sizeof(flagbuf) );
         continue;
      }
      else if( *arg == ')' && mode == 2 )
      {
         arg++;
         mode = 1;
         ptr = arg_no_flags + strlen( arg_no_flags );
         cFlag->params = strdup( parambuf );
         memset( &parambuf[0], 0, sizeof(parambuf) );
         continue;
      }
      else if( isspace( *arg ) && mode == 3 )
      {
         mode = 1;
         ptr = arg_no_flags + strlen( arg_no_flags );
         cFlag = create_flag( flagbuf );
         AttachToList( cFlag, flags );
         memset( &flagbuf[0], 0, sizeof(flagbuf) );
      }
      *ptr++ = *arg++;
   }
   if( mode == 3 )
   {
      cFlag = create_flag( flagbuf );
      AttachToList( cFlag, flags );
   }
   return;
}

void free_flag_list( LIST *flag_list )
{
   CMD_FLAG *cmdFlag;
   ITERATOR Iter;

   AttachIterator( &Iter, flag_list );
   while( ( cmdFlag = (CMD_FLAG *)NextInList( &Iter ) ) != NULL )
      free_flag( cmdFlag );
   DetachIterator( &Iter );
   FreeList( flag_list );
   return;
}

void free_flag( CMD_FLAG *cmdFlag )
{
   if( cmdFlag->flag )
      free( cmdFlag->flag );
   free( cmdFlag );
   return;
}

CMD_FLAG *get_flag( LIST *flag_list, const char *flag )
{
   CMD_FLAG *cmdFlag;
   ITERATOR Iter;

   AttachIterator( &Iter, flag_list );
   while( ( cmdFlag = (CMD_FLAG *)NextInList( &Iter ) ) != NULL )
      if( !strcmp( cmdFlag->flag, flag ) )
         break;
   DetachIterator( &Iter );


   return cmdFlag;
}
