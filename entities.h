/* File entities.h
   This is the header file for entities.c
   Written by Davenge */

extern LIST * world_entities; /* anything instanced is an entity */

struct entity
{
   D_SOCKET *dsock;
   void *content;
   int type;
   char *name;
   char *short_descr;
   char *long_descr;
   LIST *coordinates_occupied;
   LIST *events;
   LIST *commands;
};


/* creation */
ENTITY *init_entity( void );
void clear_entity( ENTITY *ent );

/* deletion */
void free_entity( ENTITY *ent );
void free_entity_list( LIST *entities );

/* i/o */
void fwrite_entity_data( ENTITY *ent, FILE *fp );
ENTITY *fread_entity_data( FILE *fp );

/*******************
 * Utility Methods *
 *******************/
void wrap_entity( void *passed, int type );
bool shift_entity( ENTITY *ent, int x, int y, int z );
