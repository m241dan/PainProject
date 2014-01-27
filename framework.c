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

   switch( fWork->type )
   {
      case ROOM_FRAME:
         fWork->id = create_new_id( dMob, ROOM_FRAME );
         fWork->content = create_rFramework();
         break;
   }

   AttachToList( fWork, frameworks );

   return fWork;
}

/* deletion */
void free_framework( FRAMEWORK *frame )
{
   switch( frame->type )
   {
      case ROOM_FRAME:
         free_rFramework( (R_FRAMEWORK *)frame->content );
         break;
   }
   free_i_id( frame->id );
   free( frame );
   return;
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

/* deletion */
void free_rFramework( R_FRAMEWORK *rFrame )
{
   free( rFrame->title );
   free( rFrame->description );
   free( rFrame );
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
   bool found, done = FALSE;

   if( ( fp = fopen( location, "r" ) ) == NULL )
   {
      bug( "%s: cannot open %s to read from.", __FUNCTION__, location );
      return FALSE;
   }

   word = ( feof( fp ) ? FILE_TERMINATOR : fread_word( fp ) );
   if( strcmp( word, "#FRAMEWORK" ) )
   {
      bug( "%s: attempting to read a file that isn't tagged as a framework.", __FUNCTION__ );
      fclose( fp );
      return FALSE;
   }
   while( !done )
   {
      found = FALSE;

      if( word[0] != '#' || word[1] == '\0' || !word[1] )
      {
         bug( "%s: getting bad file format, word is %s", __FUNCTION__, word );
         free_framework( frame );
         return FALSE;
      }
      switch( word[1] )
      {
         case 'O':
            if (!strcasecmp(word, "EOF")) {done = TRUE; found = TRUE; break;}
            break;
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
         free_framework( frame );
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
   if( frame->id )
      fwrite_i_id( frame->id, fp );
   fprintf( fp, "#END\n" );
   return;
}

void fread_framework( FRAMEWORK *frame, FILE *fp )
{
   char *word;
   bool found, done = FALSE;

   word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   while( !done )
   {
      found = FALSE;
      switch( word[0] )
      {
         case '#':
            if( !strcasecmp( word, "#END" ) ){ done = TRUE; found = TRUE; break; }
            break;
         case 'T':
            IREAD( "Type", frame->type );
            break;
      }
      if( !found )
      {
         bug( "%s: bad file format %s", __FUNCTION__, word );
         free_framework( frame );
         return;
      }
      if( !done )
         word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   }
   return;
}

/* data specific to room frameworks */
void fwrite_rFramework( R_FRAMEWORK *rFrame, FILE *fp )
{
   fprintf( fp, "#ROOMFRAME\n" );
   fprintf( fp, "Title     %s~\n", rFrame->title );
   fprintf( fp, "Descr     %s~\n", rFrame->description );
   fprintf( fp, "#END\n" );
   return;
}
R_FRAMEWORK *fread_rFramework( FILE *fp )
{
   R_FRAMEWORK *rFrame;
   char *word;
   bool found, done = FALSE;

   CREATE( rFrame, R_FRAMEWORK, 1 );

   word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   while( !done )
   {
      switch( word[0] )
      {
         case '#':
            if( strcasecmp( "#END", word ) ) { done = TRUE; found = TRUE; break; }
            break;
         case 'D':
            SREAD( "Descr", rFrame->description );
            break;
         case 'T':
            SREAD( "Title", rFrame->title );
            break;
      }
      if( !found )
      {
         bug( "%s: bad file format %s.", __FUNCTION__, word );
         free_rFramework( rFrame );
         return NULL;
      }
      if( !done )
         word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   }
   return rFrame;
}


/* -checking- */

bool valid_ftype( int type )
{
   if( type >= MAX_FRAMEWORK || type < 0 )
      return FALSE;
   return TRUE;
}

