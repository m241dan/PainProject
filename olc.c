/* File olc.c
   All methods pertaining to the olc go here
   Written by Davenge */

#include "mud.h"

LIST *workspaces = NULL;


/*******************
 * Utility Methods *
 *******************/

bool check_work( D_MOBILE *dMob )
{
   if( !dMob->workspace )
   {
      text_to_mobile( dMob, "You have no workspace open.\r\n" );
      return FALSE;
   }
   return TRUE;
}

void load_workspaces( void )
{
   workspaces = AllocList();

}

WORKSPACE *load_workspace( char *location )
{
   FILE *fp;
   WORKSPACE *work;

   if( ( fp = fopen( location, "r" ) ) == NULL )
      return NULL;

   CREATE( work, WORKSAPCE, 1 );

   
}

void add_frame_to_workspace( FRAMEWORK *frame, D_MOBILE *dMob )
{
   if( !dMob->workspace || !dMob->workspace->contents )
   {
      bug( "%s: NULL workspace or workspace content for %s.", __FUNCTION__, dMob->name );
      return;
   }
   AttachToList( frame, dMob->workspace->contents );
   return;
}
