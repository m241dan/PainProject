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
   room->title = NULL;
   room->description = NULL;
   room->inside = FALSE;
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
   room->ent_wrapper->name = strdup( fWork->name );
   room->ent_wrapper->short_descr = strdup( fWork->short_descr );
   room->ent_wrapper->long_descr = strdup( fWork->long_descr );
   room->title = strdup( rFrame->title );
   room->description = strdup( rFrame->description );

   room->id = create_new_id( dMob, ROOM_ENTITY );
   room->framework = fWork;
   return room;
}

/* deletion */
void free_room( ROOM *room )
{
   room->framework = NULL;

   if( room->ent_wrapper )
      free_entity( room->ent_wrapper );
   if( room->id )
      free_i_id( room->id );
   if( room->ent_wrapper )
      free_entity( room->ent_wrapper );
   free( room );
   return;
}

void delete_room( ROOM *room )
{
   char location[MAX_BUFFER];

   if( !room )
   {
      bug( "%s: passed a NULL room.", __FUNCTION__ );
      return;
   }

   mud_printf( location, "../instances/rooms/r%d.instance", room->id->id );
   if( !unlink( location ) )
      free_room( room );
   else
      bug( "%s: could not delete the instance of %s, ID Number %d.", __FUNCTION__, room->ent_wrapper->name, room->id->id );

   return;
}
