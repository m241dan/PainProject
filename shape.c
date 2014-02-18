/* shape.c written by Davenge */

#include "mud.h"

/* creation */
SHAPE *init_shape( void )
{
   SHAPE *shape;

   CREATE( shape, SHAPE, 1 );
   clear_shape( shape );
   shape->offsets = AllocList();
   return shape;
}

void clear_shape( SHAPE *shape )
{
   shape->name = NULL;
   shape->max_x = 0;
   shape->max_y = 0;
   shape->max_z = 0;
   return;
}

/* deletion */
void free_shape( SHAPE *shape )
{
   if( shape->name )
      free( shape );
   free_offset_list( shape->offsets );
   free( shape );
   return;
}

/* i/o */
bool load_shapes( void )
{
   SHAPE *shape;
   char dir_name[MAX_BUFFER];
   char location[MAX_BUFFER];
   DIR *directory;
   struct dirent *entry;

   mud_printf( dir_name, "../shapes/" );
   if( ( directory = opendir( dir_name ) ) == NULL )
   {
      bug( "%s: could no load %s directory.", __FUNCTION__, dir_name );
      return FALSE;
   }

   for( entry = readdir( directory ); entry; entry = readdir( directory ) )
   {
      if( !string_contains( entry->d_name, ".shape" ) )
         continue;

      shape = init_shape();
      mud_printf( location, "%s%s", dir_name, entry->d_name );
      if( !load_shape( location, shape ) )
      {
         free_shape( shape );
         bug( "%s; could not load framework %s from file.", __FUNCTION__, location );
         continue;
      }
      AttachToList( shape, all_shapes );
   }
   return TRUE;
}

void save_shape( SHAPE *shape )
{
   FILE *fp;
   char location[MAX_BUFFER];

   mud_printf( location, "../shapes/%s.shape", shape->name );
   if( ( fp = fopen( location, "w" ) ) == NULL )
   {
      bug( "%s: cannot open %s to write.", __FUNCTION__, location );
      return;
   }

   fwrite_shape( shape, fp );
   fclose( fp );
   return;
}

bool load_shape( const char *location, SHAPE *shape )
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
   if( strcmp( word, "#SHAPE" ) )
   {
      bug( "%s: attempting to read a file that isn't tagged as a shape.", __FUNCTION__ );
      fclose( fp );
      return FALSE;
   }

   while( !done )
   {
      found = FALSE;
      switch( word[1] )
      {
         case 'O':
            if( !strcasecmp( word, "EOF" ) ) { done = TRUE; found = TRUE; break; }
            break;
         case 'S':
            if( !strcmp( word, "#SHAPE" ) )
            {
               found = TRUE;
               if( !fread_shape( shape, fp ) )
                  found = FALSE;
               break;
            }
      }
      if( !found )
      {
         bug( "%s: word key not known, %s", __FUNCTION__, word );
         return FALSE;
      }
      if( !done )
         word = ( feof( fp ) ? FILE_TERMINATOR : fread_word( fp ) );
   }
   fclose( fp );
   return TRUE;
}

void fwrite_shape( SHAPE *shape, FILE *fp )
{
   OFFSET *off;
   ITERATOR Iter;

   fprintf( fp, "#SHAPE\n" );
   fprintf( fp, "Name       %s~\n", shape->name );
   fprintf( fp, "MaxX       %d\n", shape->max_x );
   fprintf( fp, "MaxY       %d\n", shape->max_y );
   fprintf( fp, "MaxZ       %d\n", shape->max_z );

   AttachIterator( &Iter, shape->offsets );
   while( ( off = (OFFSET *)NextInList( &Iter ) ) != NULL )
      fwrite_offset( off, fp );
   DetachIterator( &Iter );

   fprintf( fp, "#END\n" );
   return;
}

bool fread_shape( SHAPE *shape, FILE *fp )
{
   char *word;
   bool done, found = FALSE;

   word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   while( !done )
   {
      found = FALSE;
      switch( word[0] )
      {
         case '#':
            if( !strcasecmp( word, "#END" ) ) { done = TRUE; found = TRUE; break; }
            if( !strcasecmp( word, "#OFFSET" ) )
            {
               OFFSET *off;
               found = TRUE;
               if( ( off = fread_offset( fp ) ) == NULL )
                  found = FALSE;
               else
                  AttachToList( off, shape->offsets );
               break;
            }
            break;
         case 'M':
            IREAD( "MaxX", shape->max_x );
            IREAD( "MaxY", shape->max_y );
            IREAD( "MaxZ", shape->max_z );
            break;
         case 'N':
            SREAD( "Name", shape->name );
            break;
      }
      if( !found )
      {
         bug( "%s: bad file format %s", __FUNCTION__, word );
         return FALSE;
      }
      if( !done )
         word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   }
   return TRUE;
}
