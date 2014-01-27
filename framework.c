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
void save_framework( FRAMEWORK *frame )
{
   FILE *fp;
   char location[MAX_BUFFER];

   mud_printf( location, "../frameworks/%ss/%c%d.frame", framework_names[frame->type], framework_names[frame->type][0], frame->id );
   if( ( fp = fopen( location, "w" ) ) == NULL )
   {
      bug( "%s: cannot open %s to write.", __FUNCTION__, location );
      return;
   }

   fwrite_framework( frame, fp );

   switch( frame->type )
   {
      case ROOM_FRAME:
         fwrite_rFramework( (R_FRAMEWORK *)frame->content, fp );
         break;
   }
   fprintf( fp, "%s\n", FILE_TERMINATOR );
   fclose( fp );
   return;
}
bool load_framework( const char *location, FRAMEWORK *frame )
{
   FILE *fp;
   char *word;
   bool false, done = FALSE;

   if( ( fp = fopen( location, "r" ) ) == NULL )
   {
      bug( "%s: cannot open %s to read from.", __FUNCTION__, location );
      return;
   }

   word = fread_word( fp );
   if( strcmp( word, "#FRAMEWORK" ) )
   {
      bug( "%s: attempting to read a file that isn't tagged as a framework.", __FUNCTION__ );
      fclose( fp );
      return FALSE;
   }
   while( !done )
   {
      found = FALSE;

      if( word[0] != '#' || word[1] = '\0' || !word[1] )
      {
         bug( "%s: getting bad file format, word is %s", __FUNCTION__, word );
         return FALSE;
      }
      switch( word[1] )
      {
         case 'F':
            if( !strcmp( word, "#FRAMEWORK" ) )
            {
               found = TRUE;
               fread_framework( frame, fp );
               break;
            }
            break;
         case 'R':
            if( !strcmp( word, "#ROOMFRAME" ) )
            {
               found = TRUE;
               frame->content = fread_rFramework( fp );
               break;
            }
            break;
      }
      if( !found )
      {
         bug( "%s: word key not known, %s", __FUNCTION__, word );
         return FALSE;
      }
      if( !done )
         word = fread_word( fp );
   }
   fclose( fp );
   return TRUE;
}


/* general framework data */
void fwrite_framework( FRAMEWORK *frame, FILE *fp )
{
   fprintf( fp, "#FRAMEWORK\n" );
   fprintf( fp, "Type         %d\n", frame->type );
   fprintf( fp, "CreatedBy    %s~\n", frame->created_by );
   fprintf( fp, "ModifiedBy   %s~\n", frame->modified_by );
   
}

void fread_framework( FRAMEWORK *frame, FILE *fp )
{

}

/* data specific to room frameworks */
void fwrite_rFramework( R_FRAMEWORK *rFrame, FILE *fp )
{
   fprintf( fp, "#ROOMFRAME\n" );
   fprintf( fp, "Title     %s~\n", rFrame->title );
   fprintf( fp, "Descr     %s~\n", rFrame->description );
   fprintf( fp, "#END\n" );
}
R_FRAMEWORK *fread_rFramework( FILE *fp )
{
   return;
}


/* -checking- */

bool valid_ftype( int type )
{
   if( type >= MAX_FRAMEWORK || type < 0 )
      return FALSE;
   return TRUE;
}

