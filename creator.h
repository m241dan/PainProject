/* Creator.h Written by Davenge */

struct creator
{
   SHAPE *cShape;
   ENTITY *to_create;
};


/* creation */
CREATOR *init_creator( void );
void clear_creator( CREATOR *creator );

/* deletion */
void free_creator( CREATOR *creator );
