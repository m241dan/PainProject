/* File entities.c
   All methods pertaining to entities go here
   Written by Davenge */

#include "mud.h"

/* creation */
ENTITY *init_entity( void )
{
   ENTITY *ent;

   CREATE( ent, ENTITY, 1 );
   clear_entity( ent );
   return ent;
}

void clear_entity( ENTITY *ent )
{
   ent->content = NULL;
   ent->type = -1;
   return;
}

/* deletion */
void free_entity( ENTITY *ent )
{
   ent->content = NULL;
   free( ent );
   return;
}
/*******************
 * Utility Methods *
 *******************/

void wrap_entity( void *passed, int type )
{
   ENTITY *ent = init_entity();

   ent->type = type;
   switch( type )
   {
      case MOBILE_ENTITY:
         ((D_MOBILE *)passed)->ent_wrapper = ent;
         break;
   }
   AttachToList( ent, world_entities );
   return;
}
