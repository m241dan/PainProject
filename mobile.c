/* File mobile.c
   All methods pertaining to the mobile go here
   Written by Davenge */

#include <ctype.h>
#include "mud.h"

/* Method to save any mobile to its proper place */

void save_mobile( D_MOBILE *dMob)
{
   switch( dMob->level )
   {
      default:
         text_to_mobile( dMob, "Unknown mobile level, cannot save." );
         bug( "%s: %s attempting to save with %d level.", __FUNCTION__, dMob->name, dMob->level );
         return;
      case LEVEL_PLAYER:
         save_player( dMob, FALSE );
         break;
   }
   return;
}

void save_player( D_MOBILE *dMob, bool New )
{
   char pName[MAX_BUFFER];
   char aName[MAX_BUFFER];
   char pfile[MAX_BUFFER];
   FILE *fp;

   if( !dMob|| !dMob->name || dMob->name[0] == '\0' )
   {
      bug( "%s: attempting to save a mobile with no name.", __FUNCTION__ );
      return;
   }

   if( !dMob->account || !dMob->account->name || dMob->account->name[0] == '\0' )
   {
      bug( "%s: %s attempting to save with no account.", __FUNCTION__, dMob->name );
      return;
   }

   if( !New && !dMob->loaded )
   {
      bug( "%s: trying to save an unfully loaded mobile named %s.", __FUNCTION__, dMob->name );
      return;
   }

   strcpy( pName, dMob->name );
   strcpy( aName, dMob->account->name );
   capitalize_orig( pName );
   capitalize_orig( aName );

   snprintf( pfile, MAX_BUFFER, "../accounts/%s/%s.pfile", aName, pName );
   /* open the pfile so we can write to it */
   if ((fp = fopen(pfile, "w")) == NULL)
   {
      puts( pfile );
      bug( "%s: Unable to write to %s's pfile", __FUNCTION__, dMob->name);
      return;
   }

   /* dump the players data into the file */

   /* this is the data saved for a partial load */
   fprintf(fp, "Name            %s~\n", dMob->name);
   fprintf(fp, "Level           %d\n",  dMob->level);
   fprintf(fp, "Password        %s~\n", dMob->password);
   fprintf(fp, "Race            %d\n",  dMob->race);

   /* this is the data for a full load */

   /* terminate the file */
   fprintf(fp, "%s\n", FILE_TERMINATOR);
   fclose(fp);
}

/* unload a mobile, with options */

void unload_mobile( D_MOBILE *dMob, bool partial )
{
   if( !dMob->loaded && partial ) /* a character already logged out and want a partial unload, do nothing */
      return;
   else if( dMob->loaded && partial ) /* logging out of a character to an account */
      free_mobile_game_data( dMob );
   else if( dMob->loaded && !partial ) /* character logged in and we want to completely quit the game */
   {
      free_mobile_game_data( dMob );
      free_mobile_account_data( dMob );
      PushStack( dMob, dmobile_free );
      return;
   }
   else if( !dMob->loaded && !partial ) /* this would be quitting the game from an account and freeing up the LIST *character in account structure */
   {
      free_mobile_account_data( dMob );
      PushStack( dMob, dmobile_free );
      return;
   }
   return;
}

void alloc_mobile_lists( D_MOBILE *dMob ) /* alloc the dMobiles lists */
{
   dMob->commands = AllocList();
   dMob->events = AllocList();
   return;
}

void free_mobile_lists( D_MOBILE *dMob ) /* deallocate the dMobiles list memory */
{
   FreeList( dMob->commands );
   FreeList( dMob->events );
   return;
}

/* Free all the data related to playing the game only */
void free_mobile_game_data(D_MOBILE *dMob)
{
   if( dMob->loaded )
   {
      DetachFromList(dMob, dmobile_list);

      if (dMob->socket) dMob->socket->player = NULL;

      /* Free Up Event List */
      clear_mobile_event_list( dMob );
      /* Free up Comand List */
      clear_mobile_command_list( dMob );
      /* Now for the List's Memory */
      free_mobile_lists( dMob );
   }
   return;
}

/* Free all dMob account data */
void free_mobile_account_data( D_MOBILE *dMob )
{
   free( dMob->name );
   free( dMob->password );
   return;
}

void clear_mobile(D_MOBILE *dMob, bool partial )
{
   dMob->name         =  NULL;
   dMob->password     =  NULL;
   dMob->level        =  LEVEL_PLAYER;
   dMob->race         =  RACE_HUMAN;

   if( !partial )
      alloc_mobile_lists( dMob );
}

void load_player( ACCOUNT *account, char *player, bool partial, D_MOBILE *dMob )
{
  FILE *fp;
  char pfile[MAX_BUFFER];
  char pName[MAX_BUFFER];
  char *word;
  bool done = FALSE, found;
  int i, size;

  pName[0] = toupper(player[0]);
  size = strlen(player);
  for (i = 1; i < size && i < MAX_BUFFER - 1; i++)
    pName[i] = tolower(player[i]);
  pName[i] = '\0';

  /* open the pfile so we can write to it */
  snprintf(pfile, MAX_BUFFER, "../accounts/%s/%s", account->name, pName);
  if ((fp = fopen(pfile, "r")) == NULL)
    return;

  /* create new mobile data */
  if( !dMob )
  {
     if( StackSize( dmobile_free ) <= 0 )
        CREATE( dMob, D_MOBILE, 1 );
     else
       dMob = (D_MOBILE *)PopStack( dmobile_free );

     clear_mobile(dMob, partial);
  }

  /* load data */
  word = fread_word(fp);
  while (!done)
  {
    found = FALSE;
    switch (word[0])
    {
      case 'E':
        if (!strcasecmp(word, "EOF")) {done = TRUE; found = TRUE; break;}
        break;
      case 'L':
        IREAD( "Level",     dMob->level     );
        break;
      case 'N':
        SREAD( "Name",      dMob->name      );
        break;
      case 'P':
        SREAD( "Password",  dMob->password  );
        break;
      case 'R':
        if( !strcmp( word, "Race" ) )
        {
           found = TRUE;
           dMob->race = fread_number( fp );
           if( partial )
           {
              done = TRUE;
              dMob->loaded = FALSE;
           }
           break;
        }
        break;
    }
    if (!found)
    {
      bug("Load_player: unexpected '%s' in %s's pfile.", word, player);
      unload_mobile( dMob, FALSE );
      dMob = NULL;
      return;
    }

    /* read one more */
    if (!done) word = fread_word(fp);
  }

  fclose(fp);
  return;
}

void load_mobile_commands( D_MOBILE *dMob )
{
   COMMAND *com;
   int x;

   clear_mobile_command_list( dMob ); /* clear it out before we lost new commands */

   for( x = 0; tabCmd[x].cmd_name[0] != '\0'; x++ ) /* load the new commands that fit our criteria */
      if( tabCmd[x].state == STATE_PLAYING && tabCmd[x].level <= 2 )
      {
         com = copy_command( tabCmd[x] );
         AttachToList( com, dMob->commands );
      }
   return;
}

void clear_mobile_command_list( D_MOBILE *dMob )
{
   COMMAND *com;
   ITERATOR Iter;

   if( !dMob->commands )
      return;

   AttachIterator(&Iter, dMob->commands );
   while( ( com = (COMMAND *)NextInList(&Iter) ) != NULL )
      free_command( com );
   DetachIterator(&Iter);
   return;
}

void clear_mobile_event_list( D_MOBILE *dMob )
{
   EVENT_DATA *pEvent;
   ITERATOR Iter;

   if( !dMob->events )
      return;

   AttachIterator(&Iter, dMob->events);
   while ((pEvent = (EVENT_DATA *) NextInList(&Iter)) != NULL)
     dequeue_event(pEvent);
   DetachIterator(&Iter);
   return;
}
