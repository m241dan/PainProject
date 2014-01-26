/* File framework.c
   All methods pertaining to framework.c go here
   Written by Davenge */

#include "mud.h"

LIST *frameworks = NULL;

/*******************
 * Utility Methods *
 *******************/

/* -general- */
/* creation */
FRAMEWORK *create_framework( D_MOBILE *dMob, int type )
{
   FRAMEWORK *fWork;

   if( !valid_ftype( type ) )
   {
      bug( "%s: Bad framework type passed. User: %s", __FUNCTION__, dMob->name );
      return NULL;
   }

   CREATE( fWork, FRAMEWORK, 1 );
   fWork->type = type;
   fWork->created_on = ctime( &current_time );
   fWork->last_modified = ctime( &current_time );
   fWork->created_by = strdup( dMob->name );
   fWork->modified_by = strdup( dMob->name );

   switch( fWork->type )
   {
      case ROOM_FRAME:
         fWork->id = create_new_id( ROOM_FRAME );
         fWork->content = create_rFramework();
         break;
   }

   AttachToList( fWork, frameworks );

   return fWork;
}

/* i/o */

/* -room specific- */
/* creation */
R_FRAMEWORK *create_rFramework( void )
{
   R_FRAMEWORK *rFrame;

   CREATE( rFrame, R_FRAMEWORK, 1 );
   rFrame->title = strdup( "new room" );
   rFrame->description = strdup( "none" );

   return rFrame;
}

/* i/o */
R_FRAMEWORK *load_rFramework( char *location )
{
   return NULL;
}

void fread_rFramework( R_FRAMEWORK *rFrame, FILE *fp )
{
   return;
}

void save_rFramework( R_FRAMEWORK *rFrame )
{
   return;
}

void fwrite_rFramework( R_FRAMEWORK *rFrame, FILE *fp )
{
   fprintf( fp, "#ROOMFRAME\n" );
   fprintf( fp, "Title     %s~\n", rFrame->title );
   fprintf( fp, "Descr     %s~\n", rFrame->description );
   fprintf( fp, "#END\n" );
}

/* -checking- */

bool valid_ftype( int type )
{
   if( type >= MAX_FRAMEWORK || type < 0 )
      return FALSE;
   return TRUE;
}

