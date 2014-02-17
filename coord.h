/* File coord.h
   This is the header file for mobile.c
   Written by Davenge */


extern LIST *coord_map[COORD_HASH_KEY][COORD_HASH_KEY][COORD_HASH_KEY];

struct coordinate
{
   COORD *connected[MAX_DIRECTION];
   LIST *entities;
   int pos_x;
   int pos_y;
   int pos_z;
   ROOM *room;
   ENTITY *fill;
};

/*******************
 * Utility Methods *
 *******************/

/* creation */
COORD *init_coord( void );
void clear_coord( COORD *coord );
COORD *create_coord( int x, int y, int z );

/* deletion */
void free_coord( COORD *coord );
void free_coord_list( LIST *coords );


COORD *get_coord( int x, int y, int z );
void link_coordinate( COORD *coordinate );
bool check_coord( int x, int y, int z );
int get_coord_hash( int x );
bool same_coord( COORD *coordinate, int x, int y, int z );
int get_directional_opposite( int dir );

