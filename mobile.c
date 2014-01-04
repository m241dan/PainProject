/* File mobile.c
   All methods pertaining to the mobile go here
   Written by Davenge */

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
         save_player( dMob);
         break;
   }
   return;
}

void save_player( D_MOBILE *dMob)
{
   char pName[MAX_BUFFER];
   char aName[MAX_BUFFER];
   char pfile[MAX_BUFFER];
   FILE *fp;
   int size, i;

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

   snprintf( pfile, MAX_BUFFER, "../acccounts/%s/%s.pfile", pfile, pName );
  /* open the pfile so we can write to it */
  snprintf(pfile, MAX_BUFFER, "../players/%s.pfile", pName);
  if ((fp = fopen(pfile, "w")) == NULL)
  {
    bug("Unable to write to %s's pfile", dMob->name);
    return;
  }

  /* dump the players data into the file */
  fprintf(fp, "Name            %s~\n", dMob->name);
  fprintf(fp, "Level           %d\n",  dMob->level);
  fprintf(fp, "Password        %s~\n", dMob->password);
  fprintf(fp, "Race            %d\n",  dMob->race);

  /* terminate the file */
  fprintf(fp, "%s\n", FILE_TERMINATOR);
  fclose(fp);

}

