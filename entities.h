/* File entities.h
   This is the header file for entities.c
   Written by Davenge */

extern LIST * world_entities; /* anything instanced is an entity */

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
void free_entity_list( LIST *entities );
/*******************
 * Utility Methods *
 *******************/
void wrap_entity( void *passed, int type );
