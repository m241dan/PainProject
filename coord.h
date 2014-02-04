/* File coord.h
   This is the header file for mobile.c
   Written by Davenge */


extern LIST *coord_map[MAX_COORD_HASH];

struct coordinate
{
   COORD *exits[MAX_DIRECTION];
   LIST *entities;
   /* ROOM_DATA *room */
   int pos_x;
   int pos_y;
   int pos_z;
};

/*******************
 * Utility Methods *
 *******************/
COORD *create_coord( int x, int y, int z );
COORD *get_coord( int x, int y, int z );
void link_coordinate( COORD *coordinate );
bool check_coord( int x, int y, int z );
int get_coord_hash( int x );
bool same_coord( COORD *coordinate, int x, int y, int z );
int get_directional_opposite( int dir );

