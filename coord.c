/* File coord.c
   All methods pertaining to the coordintes go here
   Written by Davenge */

#include "mud.h"

/*******************
 * Utility Methods *
 *******************/

COORD *create_coord( int x, int y, int z )
{
   COORD *coordinate;
   int hash = get_coord_hash( x );

   if( check_coord( x, y, z ) )
   {
      bug( "%s: attempting to create a coord that already exists, x: %d y: %d z: %d", __FUNCTION__, x, y, z );
      return NULL;
   }

   CREATE( coordinate, COORD, 1 );
   coordinate->pos_x = x;
   coordinate->pos_y = y;
   coordinate->pos_z = z;
   coordinate->entities = AllocList();
   link_coordinate( coordinate );
   AttachToList( coordinate, coord_map[hash] );
   return coordinate;
}

COORD *get_coord( int x, int y, int z )
{
   COORD *coordinate;
   ITERATOR Iter;
   int hash = get_coord_hash( x );

   AttachIterator( &Iter, coord_map[hash] );
   while( ( coordinate = (COORD *)NextInList( &Iter ) ) != NULL )
      if( same_coord( coordinate, x, y, z ) )
         return coordinate;
   DetachIterator( &Iter );
   return NULL;
}

void link_coordinate( COORD *coordinate )
{
   int x;

   /* link internally */
   coordinate->exits[DIR_NORTH] = get_coord( coordinate->pos_x, ( coordinate->pos_y + 1 ), coordinate->pos_z );
   coordinate->exits[DIR_EAST] = get_coord( ( coordinate->pos_x + 1 ), coordinate->pos_y, coordinate->pos_z );
   coordinate->exits[DIR_SOUTH] = get_coord( coordinate->pos_x, ( coordinate->pos_y -1 ), coordinate->pos_z );
   coordinate->exits[DIR_WEST] = get_coord( ( coordinate->pos_x - 1 ), coordinate->pos_y, coordinate->pos_z );
   coordinate->exits[DIR_UP] = get_coord( coordinate->pos_x, coordinate->pos_y, ( coordinate->pos_z + 1 ) );
   coordinate->exits[DIR_DOWN] = get_coord( coordinate->pos_x, coordinate->pos_y, ( coordinate->pos_z -1 ) );

   /* link externally, back to itself */
   for( x = 0; x < MAX_DIRECTION; x++ )
      if( coordinate->exits[x] ) 
         coordinate->exits[x]->exits[get_directional_opposite( x )] = coordinate;
}

bool check_coord( int x, int y, int z )
{
   COORD *coordinate;
   ITERATOR Iter;
   int hash = get_coord_hash( x );

   AttachIterator( &Iter, coord_map[hash] );
   while( ( coordinate = (COORD *)NextInList( &Iter ) ) != NULL )
      if( same_coord( coordinate, x, y, z ) )
         return TRUE;
   DetachIterator( &Iter );
   return FALSE;
}

/*****************
 * Macro Methods *
 *****************/

int get_coord_hash( int x )
{
   return abs(x) % MAX_COORD_HASH;
}

bool same_coord( COORD *coordinate, int x, int y, int z )
{
   return ( coordinate->pos_x == x && coordinate->pos_y == x && coordinate->pos_z == z );
}

int get_directional_opposite( int dir )
{
   switch( dir )
   {
      case DIR_NORTH:
      case DIR_EAST:
         return ( dir + 2 );
      case DIR_SOUTH:
      case DIR_WEST:
         return ( dir - 2 );
      case DIR_UP:
         return ( dir + 1 );
      case DIR_DOWN:
         return ( dir - 1 );
   }
   return -1;
}
