/* File entities.c
   All methods pertaining to entities go here
   Written by Davenge */

#include "mud.h"

/*******************
 * Utility Methods *
 *******************/

void wrap_entity( void *passed, int type )
{
   ENTITY *ent;

   CREATE( ent, ENTITY, 1 );
   ent->content = passed;
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
