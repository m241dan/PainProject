/* File entities.c
   All methods pertaining to entities go here
   Written by Davenge */

#include "mud.h"

/* creation */
ENTITY *init_entity( void )
{
   ENTITY *ent;

   CREATE( ent, ENTITY, 1 );
   clear_entity( ent );
   ent->coordinates_occupied = AllocList();
   return ent;
}

void clear_entity( ENTITY *ent )
{
   ent->socket = NULL;
   ent->content = NULL;
   ent->type = -1;
   ent->name = NULL;
   ent->short_descr = NULL;
   ent->long_descr = NULL;
   return;
}

/* deletion */
void free_entity( ENTITY *ent )
{
   COORD *coord;
   ITERATOR Iter;

   DetachFromList( ent, world_entities );
   if( ent->name )
      free( ent->name );
   if( ent->short_descr )
      free( ent->short_descr );
   if( ent->long_descr )
      free( ent->long_descr );
   ent->content = NULL;

   AttachIterator( &Iter, ent->coordinates_occupied );
   while( ( coord = (COORD *)NextInList( &Iter ) ) != NULL )
      DetachFromList( coord, ent->coordinates_occupied );
   DetachIterator( &Iter );
   FreeList( ent->coordinates_occupied );

   free( ent );
   return;
}
void free_entity_list( LIST *entities )
{
   ENTITY *ent;
   ITERATOR Iter;

   AttachIterator( &Iter, entities );
   while( ( ent = (ENTITY *)NextInList( &Iter ) ) != NULL )
   {
      DetachFromList( ent, entities );
      free_entity( ent );
   }
   DetachIterator( &Iter );
   FreeList( entities );

   return;
}

/* i/o */
void fwrite_entity_data( ENTITY *ent, FILE *fp )
{
   fprintf( fp, "#ENTITY\n" );
   fprintf( fp, "Name         %s~\n", ent->name );
   fprintf( fp, "ShortDescr   %s~\n", ent->short_descr );
   fprintf( fp, "LongDescr    %s~\n", ent->long_descr );
   fprintf( fp, "#END\n" );
}
ENTITY *fread_entity_data( FILE *fp )
{
   ENTITY *ent = init_entity();
   char *word;
   bool found, done = FALSE;

   word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   while( !done )
   {
      switch( word[0] )
      {
         case '#':
            if( !strcasecmp( "#END", word ) ) { done = TRUE; found = TRUE; break; }
            break;
         case 'L':
            SREAD( "LongDescr", ent->long_descr );
            break;
         case 'N':
            SREAD( "Name", ent->name );
            break;
         case 'S':
            SREAD( "ShortDescr", ent->short_descr );
            break;
      }
      if( !found )
      {
         bug( "%s: bad file format %s.", __FUNCTION__, word );
         free_entity( ent );
         return NULL;
      }
      if( !done )
         word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   }
   return ent;
}

/*******************
 * Utility Methods *
 *******************/

void wrap_entity( void *passed, int type )
{
   ENTITY *ent = init_entity();

   ent->type = type;
   ent->content = passed;
   switch( type )
   {
      case MOBILE_ENTITY:
         ((D_MOBILE *)passed)->ent_wrapper = ent;
         break;
   }
   AttachToList( ent, world_entities );
   return;
}

bool can_shift_entity( ENTITY *ent, int x, int y, int z )
{
   return FALSE;
/*
   COORD *coord, *to_coord;
   ITERATOR Iter;
   bool can_shift = TRUE;

   AttachIterator( &Iter, ent->coordinates_occupied );
   while( ( coord = (COORD *)NextInList( &Iter ) ) != NULL )
   {
      if( ( to_coord = get_coord( coord->pos_x + x, coord->pos_y + y, coord->pos_z + z ) ) == NULL )
      {
         if( x = 0 && y = 0 )
            continue;
         else
         {
            ent_printf( 
            return FALSE;
   }
*/
}
