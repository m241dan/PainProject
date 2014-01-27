/* File id.c
   All methods pertaining to the id.c go here
   Written by Davenge */

#include "mud.h"

/*******************
 * Utility Methods *
 *******************/

/* creation */

ID_HANDLER *init_id_handler( int type )
{
   ID_HANDLER *handler;

   CREATE( handler, ID_HANDLER, 1 );
   handler->free_ids = AllocList();
   handler->type = type;
   handler->id = 0;
   return handler;
}

I_ID *create_raw_id( int id, const char *create, const char *modify )
{
   I_ID *i_id;

   CREATE( i_id, I_ID, 1 );
   i_id->id = id;
   i_id->created_on = strdup( create );
   i_id->last_modified = strdup( modify );
   return i_id;
}

I_ID *create_new_id( int type )
{
   I_ID *id;

   switch( type )
   {
      case ROOM_ENTITY:
         if( ( id = check_free( rid_handler ) ) == NULL )
            id = create_raw_id( get_top_id( rid_handler ), ctime( &current_time ), ctime( &current_time ) );
         break;
   }
   return id;
}

/* i/o */

void save_id_handler( ID_HANDLER *handler )
{

}

bool load_id_handler( ID_HANDLER *handler )
{

}

void fwrite_id_handler( ID_HANDLER *handler, FILE *fp )
{

}

void fread_id_handler( ID_HANDLER *handler, FILE *fp )
{

}

void fwrite_i_id( I_ID *id, FILE *fp )
{

}

void fread_i_id( I_ID *id, FILE *fp )
{

}

/* utility */
I_ID *check_free( ID_HANDLER *handler )
{
   ITERATOR Iter;
   I_ID *id;

   if( SizeOfList( handler->free_ids ) <= 0 )
      return NULL;

   AttachIterator( &Iter, handler->free_ids );
   id = NextInList( &Iter );
   DetachFromList( id, handler->free_ids );
   DetachIterator( &Iter );
   id->created_on = ctime( &current_time );
   id->last_modified = ctime( &current_time );
   return id;
}

int get_top_id( ID_HANDLER *handler )
{
   handler->top_id += 1;
   return handler->top_id;
}
