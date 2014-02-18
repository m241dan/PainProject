/* Offset.c Written by Davenge */

#include "mud.h"

/* creation */
OFFSET *init_offset( void )
{
   OFFSET *off;

   CREATE( off, OFFSET, 1 );
   clear_offset( off );
   return off;
}

void clear_offset( OFFSET *off )
{
   off->x = 0;
   off->y = 0;
   off->z = 0;
   return;
}

/* deletion */
void free_offset( OFFSET *off )
{
   free( off );
   return;
}

void free_offset_list( LIST *offsets )
{
   OFFSET *off;
   ITERATOR Iter;

   AttachIterator( &Iter, offsets );
   while( ( off = (OFFSET *)NextInList( &Iter ) ) != NULL )
      free_offset( off );
   DetachIterator( &Iter );

   return;
}

/* i/o */
void fwrite_offset( OFFSET *off, FILE *fp )
{
   fprintf( fp, "#OFFSET\n" );
   fprintf( fp, "OffX       %d", off->x );
   fprintf( fp, "OffY       %d", off->y );
   fprintf( fp, "OffZ       %d", off->z );
   fprintf( fp, "#END\n" );
   return;
}

OFFSET *fread_offset( FILE *fp )
{
   OFFSET *off = init_offset();
   char *word;
   bool found, done = FALSE;

   word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   while( !done )
   {
      found = FALSE;
      switch( word[0] )
      {
         case '#':
            if( !strcasecmp( word, "#END" ) ) { found = TRUE; done = TRUE; break; }
            break;
         case 'O':
            IREAD( "OffX", off->x );
            IREAD( "OffY", off->y );
            IREAD( "OffZ", off->z );
            break;
      }
      if( !found )
      {
         bug( "%s: bad file format: %s", __FUNCTION__, word );
         free_offset( off );
         return NULL;
      }
      if( !done )
         word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   }
   return off;
}
