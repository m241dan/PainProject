/* File room.c
   All methods pertaining to the room go here
   Written by Davenge */

#include "mud.h"

/* creation */
ROOM *init_room( void )
{
   ROOM *room;

   CREATE( room, ROOM, 1 );
   clear_room( room );
   wrap_entity( room, ROOM_ENTITY );
   return room;
}

void clear_room( ROOM *room )
{
   room->framework = NULL;
   room->ent_wrapper = NULL;
   room->id = NULL;
   room->at_coord = NULL;
   room->name = NULL;
   room->short_descr = NULL;
   room->long_descr = NULL;
   room->title = NULL;
   room->description = NULL;
   room->imposed = -1;
   return;
}

ROOM *create_room( D_MOBILE *dMob, FRAMEWORK *fWork )
{
   ROOM *room;
   R_FRAMEWORK *rFrame;

   if( ( rFrame = (R_FRAMEWORK *)fWork->content ) == NULL )
   {
      bug( "%s: Framework chosen has a NULL content.", __FUNCTION__ );
      return NULL;
   }
   room = init_room();
   room->name = strdup( fWork->name );
   room->short_descr = strdup( fWork->short_descr );
   room->long_descr = strdup( fWork->long_descr );
   room->title = strdup( rFrame->title );
   room->description = strdup( rFrame->description );
   room->id = create_new_id( dMob, ROOM_ENTITY );
}
