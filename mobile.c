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
         save_player( dMob );
         break;
   }
   return;
}

void save_player( D_MOBILE *dMob )
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

void unload_mobile( D_MOBILE *dMob, bool partial )
{
   return;
}

void free_mobile(D_MOBILE *dMob)
{
   EVENT_DATA *pEvent;
   ITERATOR Iter;

   if( dMob->loaded )
   {
      DetachFromList(dMob, dmobile_list);

      if (dMob->socket) dMob->socket->player = NULL;

      AttachIterator(&Iter, dMob->events);
      while ((pEvent = (EVENT_DATA *) NextInList(&Iter)) != NULL)
        dequeue_event(pEvent);
      DetachIterator(&Iter);
      FreeList(dMob->events);
   }

   /* free allocated memory */
   free(dMob->name);
   free(dMob->password);

   PushStack(dMob, dmobile_free);
}

void clear_mobile(D_MOBILE *dMob)
{
  dMob->name         =  NULL;
  dMob->password     =  NULL;
  dMob->level        =  LEVEL_PLAYER;
}

D_MOBILE *load_player( ACCOUNT *account, char *player, bool partial )
{
  FILE *fp;
  D_MOBILE *dMob = NULL;
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
    return NULL;

  /* create new mobile data */
  if (StackSize(dmobile_free) <= 0)
     CREATE( dMob, D_MOBILE, 1 );
  else
    dMob = (D_MOBILE *) PopStack(dmobile_free);

  clear_mobile(dMob);

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
      free_mobile(dMob);
      return NULL;
    }

    /* read one more */
    if (!done) word = fread_word(fp);
  }

  fclose(fp);
  return dMob;
}
