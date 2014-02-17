/* File coord.c
   All methods pertaining to the coordintes go here
   Written by Davenge */

#include "mud.h"

/* creation */
COORD *init_coord( void )
{
   COORD *coord;

   CREATE( coord, COORD, 1 );
   clear_coord( coord );
   coord->entities = AllocList();
   return coord;
}

void clear_coord( COORD *coord )
{
   int x;

   for( x = 0; x < MAX_DIRECTION; x++ )
      coord->connected[x] = NULL;
   coord->room = NULL;
   coord->fill = NULL;
   return;
}

COORD *create_coord( int x, int y, int z )
{
   COORD *coordinate = init_coord();
   int hash_x = get_coord_hash( x );
   int hash_y = get_coord_hash( y );
   int hash_z = get_coord_hash( z );
   if( check_coord( x, y, z ) )
   {
      bug( "%s: attempting to create a coord that already exists, x: %d y: %d z: %d", __FUNCTION__, x, y, z );
      return NULL;
   }

   coordinate->pos_x = x;
   coordinate->pos_y = y;
   coordinate->pos_z = z;
   link_coordinate( coordinate );
   AttachToList( coordinate, coord_map[hash_x][hash_y][hash_z] );
   return coordinate;
}

/* deletion */
void free_coord( COORD *coord )
{
   int x;
   int hash_x = get_coord_hash( coord->pos_x );
   int hash_y = get_coord_hash( coord->pos_y );
   int hash_z = get_coord_hash( coord->pos_z );

   DetachFromList( coord, coord_map[hash_x][hash_y][hash_z] );
   for( x = 0; x < MAX_DIRECTION; x++ )
      coord->connected[x] = NULL;
   if( coord->entities )
      free_entity_list( coord->entities );
   coord->room = NULL;
   coord->fill = NULL;
}

void free_coord_list( LIST *coords )
{
   COORD *coord;
   ITERATOR Iter;

   AttachIterator( &Iter, coords );
   while( ( coord = (COORD *)NextInList( &Iter ) ) != NULL )
      free_coord( coord );
   DetachIterator( &Iter );
   FreeList( coords );

   return;
}
/*******************
 * Utility Methods *
 *******************/

COORD *get_coord( int x, int y, int z )
{
   COORD *coordinate;
   ITERATOR Iter;
   int hash_x = get_coord_hash( x );
   int hash_y = get_coord_hash( y );
   int hash_z = get_coord_hash( z );

   AttachIterator( &Iter, coord_map[hash_x][hash_y][hash_z] );
   while( ( coordinate = (COORD *)NextInList( &Iter ) ) != NULL )
      if( same_coord( coordinate, x, y, z ) )
         break;
   DetachIterator( &Iter );
   return coordinate;
}

void link_coordinate( COORD *coordinate )
{
   int x;

   /* link internally */
   coordinate->connected[DIR_NORTH] = get_coord( coordinate->pos_x, ( coordinate->pos_y + 1 ), coordinate->pos_z );
   coordinate->connected[DIR_EAST] = get_coord( ( coordinate->pos_x + 1 ), coordinate->pos_y, coordinate->pos_z );
   coordinate->connected[DIR_SOUTH] = get_coord( coordinate->pos_x, ( coordinate->pos_y -1 ), coordinate->pos_z );
   coordinate->connected[DIR_WEST] = get_coord( ( coordinate->pos_x - 1 ), coordinate->pos_y, coordinate->pos_z );
   coordinate->connected[DIR_UP] = get_coord( coordinate->pos_x, coordinate->pos_y, ( coordinate->pos_z + 1 ) );
   coordinate->connected[DIR_DOWN] = get_coord( coordinate->pos_x, coordinate->pos_y, ( coordinate->pos_z -1 ) );
   coordinate->connected[DIR_NORTH_EAST] = get_coord( ( coordinate->pos_x + 1 ), ( coordinate->pos_y + 1 ), coordinate->pos_z );
   coordinate->connected[DIR_NORTH_WEST] = get_coord( ( coordinate->pos_x - 1 ), ( coordinate->pos_y + 1 ), coordinate->pos_z );
   coordinate->connected[DIR_SOUTH_EAST] = get_coord( ( coordinate->pos_x + 1 ), ( coordinate->pos_y - 1 ), coordinate->pos_z );
   coordinate->connected[DIR_SOUTH_WEST] = get_coord( ( coordinate->pos_x - 1 ), ( coordinate->pos_y - 1 ), coordinate->pos_z );

   /* link externally, back to itself */
   for( x = 0; x < MAX_DIRECTION; x++ )
      if( coordinate->connected[x] ) 
         coordinate->connected[x]->connected[get_directional_opposite( x )] = coordinate;
}

bool check_coord( int x, int y, int z )
{
   COORD *coordinate;
   ITERATOR Iter;
   bool same = FALSE;
   int hash_x = get_coord_hash( x );
   int hash_y = get_coord_hash( y );
   int hash_z = get_coord_hash( z );

   AttachIterator( &Iter, coord_map[hash_x][hash_y][hash_z] );
   while( ( coordinate = (COORD *)NextInList( &Iter ) ) != NULL )
      if( same_coord( coordinate, x, y, z ) )
      {
         same = TRUE;
         break;
      }
   DetachIterator( &Iter );
   return same;
}

/*****************
 * Macro Methods *
 *****************/

int get_coord_hash( int x )
{
   return abs(x) % COORD_HASH_KEY;
}

bool same_coord( COORD *coordinate, int x, int y, int z )
{
   return ( coordinate->pos_x == x && coordinate->pos_y == y && coordinate->pos_z == z );
}

int get_directional_opposite( int dir )
{
   switch( dir )
   {
      case DIR_NORTH:
      case DIR_EAST:
      case DIR_NORTH_EAST:
      case DIR_NORTH_WEST:
         return ( dir + 2 );
      case DIR_SOUTH:
      case DIR_WEST:
      case DIR_SOUTH_WEST:
      case DIR_SOUTH_EAST:
         return ( dir - 2 );
      case DIR_UP:
         return ( dir + 1 );
      case DIR_DOWN:
         return ( dir - 1 );
   }
   return -1;
}
