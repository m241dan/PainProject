/*
 * This file contains all sorts of utility functions used
 * all sorts of places in the code.
 */
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

/* include main header file */
#include "mud.h"

/*
 * Check to see if a given name is
 * legal, returning FALSE if it
 * fails our high standards...
 */
bool check_name(const char *name)
{
  int size, i;

  if ((size = strlen(name)) < 3 || size > 12)
    return FALSE;

  for (i = 0 ;i < size; i++)
    if (!isalpha(name[i])) return FALSE;

  return TRUE;
}

void communicate(D_MOBILE *dMob, char *txt, int range)
{
  D_MOBILE *xMob;
  ITERATOR Iter;
  char buf[MAX_BUFFER];
  char message[MAX_BUFFER];

  switch(range)
  {
    default:
      bug("Communicate: Bad Range %d.", range);
      return;
    case COMM_LOCAL:  /* everyone is in the same room for now... */
      snprintf(message, MAX_BUFFER, "%s says '%s'.\n\r", dMob->name, txt);
      snprintf(buf, MAX_BUFFER, "You say '%s'.\n\r", txt);
      text_to_mobile(dMob, buf);
      AttachIterator(&Iter, dmobile_list);
      while ((xMob = (D_MOBILE *) NextInList(&Iter)) != NULL)
      {
        if (xMob == dMob) continue;
        text_to_mobile(xMob, message);
      }
      DetachIterator(&Iter);
      break;
    case COMM_LOG:
      snprintf(message, MAX_BUFFER, "[LOG: %s]\n\r", txt);
      AttachIterator(&Iter, dmobile_list);
      while ((xMob = (D_MOBILE *) NextInList(&Iter)) != NULL)
      {
        if (!IS_ADMIN(xMob)) continue;
        text_to_mobile(xMob, message);
      }
      DetachIterator(&Iter);
      break;
  }
}

/*
 * Loading of help files, areas, etc, at boot time.
 */
void load_muddata(bool fCopyOver)
{
  load_helps();

  /* copyover *
  if (fCopyOver)
    copyover_recover(); */
}

char *get_time()
{
  static char buf[16];
  char *strtime;
  int i;

  strtime = ctime(&current_time);
  for (i = 0; i < 15; i++)   
    buf[i] = strtime[i + 4];
  buf[15] = '\0';

  return buf;
}

/* Recover from a copyover - load players */
/*
void copyover_recover()
{     
  D_MOBILE *dMob;
  D_SOCKET *dsock;
  FILE *fp;
  char name [100];
  char host[MAX_BUFFER];
  int desc;
      
  log_string("Copyover recovery initiated");
   
  if ((fp = fopen(COPYOVER_FILE, "r")) == NULL)
  {  
    log_string("Copyover file not found. Exitting.");
    exit (1);
  }
      
  * In case something crashes - doesn't prevent reading *
  unlink(COPYOVER_FILE);
    
  for (;;)
  {  
    fscanf(fp, "%d %s %s\n", &desc, name, host);
    if (desc == -1)
      break;

    dsock = malloc(sizeof(*dsock));
    clear_socket(dsock, desc);
  
    dsock->hostname     =  strdup(host);
    AttachToList(dsock, dsock_list);
 
    * load player data *
    if ((dMob = load_player(name)) != NULL)
    {
      * attach to socket *
      dMob->socket     =  dsock;
      dsock->player    =  dMob;
  
      * attach to mobile list *
      AttachToList(dMob, dmobile_list);

      * initialize events on the player *
      init_events_player(dMob);
    }
    else * ah bugger *
    {
      close_socket(dsock, FALSE);
      continue;
    }
   
    * Write something, and check if it goes error-free *
    if (!text_to_socket(dsock, "\n\r <*>  And before you know it, everything has changed  <*>\n\r"))
    { 
      close_socket(dsock, FALSE);
      continue;
    }
  
    * make sure the socket can be used *
    dsock->bust_prompt    =  TRUE;
    dsock->lookup_status  =  TSTATE_DONE;
    dsock->state          =  STATE_PLAYING;

    * negotiate compression *
    text_to_buffer(dsock, (char *) compress_will2);
    text_to_buffer(dsock, (char *) compress_will);
  }
  fclose(fp);
}     
*/
D_MOBILE *check_reconnect(char *player)
{
  D_MOBILE *dMob;
  ITERATOR Iter;

  AttachIterator(&Iter, dmobile_list);
  while ((dMob = (D_MOBILE *) NextInList(&Iter)) != NULL)
  {
    if (!strcasecmp(dMob->name, player))
    {
      if (dMob->socket)
        close_socket(dMob->socket, TRUE);

      break;
    }
  }
  DetachIterator(&Iter);

  return dMob;
}

COMMAND *copy_command( const struct typCmd to_copy )
{
   COMMAND *copy;

   CREATE( copy, COMMAND, 1 );
   copy->cmd_name = strdup( to_copy.cmd_name );
   copy->cmd_funct = to_copy.cmd_funct;
   copy->level = to_copy.level;
   copy->state = to_copy.state;
   return copy;
}

void free_command( COMMAND *command )
{
   command->cmd_name = NULL;
   command->cmd_funct = NULL;
   free(command);
}

/* Count Colorm, how many color tags are used in a string? */
int count_color( const char *str )
{
   int x;
   int count = 0;

   if( !str || str[0] == '\0' )
      return count;

   for( x = 0; x < strlen( str ); x++ )
      if( str[x] == '#' )
         count++;

   return count;
}

bool valid_mobile( D_MOBILE *dMob )
{
   if( !dMob || !dMob->name || dMob->name[0] == '\0' )
   {
      bug( "%s: attempting to save a mobile with no name.", __FUNCTION__ );
      return FALSE;
   }

   if( dMob->level >= LEVEL_PLAYER && ( !dMob->account || !dMob->account->name || dMob->account->name[0] == '\0' ) )
   {
      bug( "%s: attempting to save with no account.", __FUNCTION__ );
      return FALSE;
   }
   return TRUE;
}
