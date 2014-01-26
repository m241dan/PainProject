/* File entities.h
   This is the header file for entities.c
   Written by Davenge */

extern LIST * world_entities;

struct entity
{
   void *content;
   int type;
};

/*******************
 * Utility Methods *
 *******************/
void wrap_entity( void *passed, int type );
