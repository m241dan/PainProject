/* File entities.h
   This is the header file for entities.c
   Written by Davenge */

extern LIST * world_entities;

typedef enum {
   MOBILE_ENTITY, ROOM_ENTITIY, MAX_ENTITY
} entity_types_enum;

struct entity
{
   void *content;
   int type;
};

/*******************
 * Utility Methods *
 *******************/
void wrap_entity( void *passed, int type );
