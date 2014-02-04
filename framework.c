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
FRAMEWORK *init_framework( int type )
{
   FRAMEWORK *frame;

   CREATE( frame, FRAMEWORK, 1 );
   frame->type = type;
   return frame;
}

FRAMEWORK *create_framework( D_MOBILE *dMob, int type )
{
   FRAMEWORK *fWork;

   if( !valid_ftype( type ) )
   {
      bug( "%s: Bad framework type passed. User: %s", __FUNCTION__, dMob->name );
      return NULL;
   }

   fWork = init_framework( type );

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
/* -general- */
bool load_frameworks( void )
{
   FRAMEWORK *fWork;
   char dir_name[MAX_BUFFER];
   char location[MAX_BUFFER];
   DIR *directory;
   struct dirent *entry;
   int x;

   for( x = 0; x < MAX_FRAMEWORK; x++ )
   {
      mud_printf( dir_name, "../frameworks/%ss/", framework_names[x] );
      puts( dir_name );
      if( ( directory = opendir( dir_name ) ) == NULL )
      {
         bug( "%s: could not load %s directory.", __FUNCTION__, dir_name );
         return FALSE;
      }

      for( entry = readdir( directory ); entry; entry = readdir( directory ) )
      {
         if( !string_contains( entry->d_name, ".frame" ) )
            continue;

         fWork = init_framework( x );
         mud_printf( location, "%s%s", dir_name, entry->d_name );
         if( !load_framework( location, fWork ) )
         {
            bug( "%s: could not load framework %s from file.", __FUNCTION__, location );
            continue;
         }
         AttachToList( fWork, all_frameworks );
      }
   }
   return TRUE;
}

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
   fwrite_i_id( frame->id, fp );
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
         case 'I':
            if( !strcmp( word, "#I_ID" ) )
            {
               found = TRUE;
               frame->id = fread_i_id( fp );
               break;
            }
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

/* -room specific- */
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


/* retrieval */
FRAMEWORK *get_frame( int type, int id )
{
   FRAMEWORK *frame;
   ITERATOR Iter;

   AttachIterator( &Iter, all_frameworks );
   while( ( frame = (FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      if( frame->type == type && frame->id->id == id )
         break;
   DetachIterator( &Iter );

   return frame;
}

/* -checking- */

bool valid_ftype( int type )
{
   if( type >= MAX_FRAMEWORK || type < 0 )
      return FALSE;
   return TRUE;
}
