/* File id.c
   All methods pertaining to the id.c go here
   Written by Davenge */

#include "mud.h"

/*******************
 * Utility Methods *
 *******************/

ID_HANDLER *init_id_handler( void )
{
   ID_HANDLER *handler;

   CREATE( handler, ID_HANDLER, 1 );
   handler->free_ids = AllocList();
   handler->top_id = 0;
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

void fwrite_id_handler( ID_HANDLER *handler, const char *location )
{
   FILE *fp;
   I_ID *id;
   ITERATOR Iter;

   if( ( fp = fopen( location, "w" ) ) == NULL )
   {
      bug( "%s: can't open file to write handler at: %s", __FUNCTION__, location );
      return;
   }

   fprintf( fp, "TopID             %d\n", handler->top_id );
   AttachIterator( &Iter, handler->free_ids );
   while( ( id = (I_ID *)NextInList( &Iter ) ) != NULL )
      fprintf( fp, "FreeID      %d %s~ %s~\n", id->id, id->created_on, id->last_modified );
   fprintf( fp, "%s\n", FILE_TERMINATOR );
   fclose( fp );
   return;
}

void fread_id_handler( ID_HANDLER *handler, const char *location )
{
   I_ID *id;
   FILE *fp;
   bool done = FALSE, found;
   char *word;

   if( !handler->free_ids )
   {
      bug( "%s: trying to load to an uninitialized handler: %s.", __FUNCTION__, location );
      return;
   }

   if( ( fp = fopen( location, "r" ) ) == NULL )
   {
      bug( "%s: can't open id_handler dat.", __FUNCTION__ );
      return;
   }

   while( !done )
   {
      found = FALSE;
      switch( word[0] )
      {
         case 'E':
            if (!strcasecmp(word, "EOF")) {done = TRUE; found = TRUE; break;}
            break;
         case 'F':
            if( !strcmp( word, "FreeID" ) )
            {
               found = TRUE;
               id = create_raw_id( fread_number( fp ), fread_string( fp ), fread_string( fp ) );
               AttachToList( id,  handler->free_ids );
               break;
            }
            break;
         case 'T':
            IREAD( "TopID", handler->top_id );
            break;
      }
      if( !found )
      {
         bug( "%s: unexpected '%s' in %s.", __FUNCTION__, word, location );
         return;
      }
      if( !done )
         word = fread_word( fp );
   }
   fclose( fp );
   return;
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
