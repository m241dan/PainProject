/* File entities.h
   This is the header file for entities.c
   Written by Davenge */

extern LIST * world_entities;

struct entity
{
   void *content;
   int type;
};


/* creation */
ENTITY *init_entity( void );
void clear_entity( ENTITY *ent );

/* deletion */
void free_entity( ENTITY *ent );

/*******************
 * Utility Methods *
 *******************/
void wrap_entity( void *passed, int type );
