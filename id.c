/* File id.c
   All methods pertaining to the id.c go here
   Written by Davenge */

#include "mud.h"

/*******************
 * Utility Methods *
 *******************/

/* creation */

bool load_id_handlers( void )
{
   int x;
   ID_HANDLER *handler;

   for( x = 0; x < MAX_STRUCT; x++ )
   {
      handler = init_id_handler( x );
      if( !load_id_handler( handler ) )
      {
         bug( "%s: could not load %s id handler.", __FUNCTION__, id_handler_names[x] );
         return FALSE;
      }
      AttachToList( handler, id_handlers );
   }
   return TRUE;
}

ID_HANDLER *init_id_handler( int type )
{
   ID_HANDLER *handler;

   CREATE( handler, ID_HANDLER, 1 );
   handler->free_ids = AllocList();
   handler->type = type;
   handler->top_id = 0;
   return handler;
}

I_ID *create_raw_id( int id )
{
   I_ID *i_id;

   CREATE( i_id, I_ID, 1 );
   i_id->id = id;
   i_id->created_on = ctime( &current_time );
   i_id->last_modified = ctime( &current_time );
   return i_id;
}

I_ID *create_new_id( D_MOBILE *dMob, int type )
{
   I_ID *id;
   ID_HANDLER *handler = get_id_handler( type );

   if( ( id = check_free( handler ) ) == NULL )
      id = create_raw_id( use_top_id( handler ) );

   id->created_by = strdup( dMob ? dMob->name : "system" );
   id->modified_by = strdup( dMob ? dMob->name : "system" );
   return id;
}

/* deletion */
void free_id_handler( ID_HANDLER *handler )
{
   I_ID *id;
   ITERATOR Iter;

   /* no other pointers to free at the moment */
   DetachFromList( handler, id_handlers );
   AttachIterator( &Iter, handler->free_ids );
   while ( ( id = (I_ID *)NextInList(&Iter) ) == NULL )
      free_i_id( id );
   DetachIterator( &Iter );

   free( handler ); /*free the memory alotted for the handler*/
   return;
}

void free_i_id( I_ID *id )
{
   free(id->created_by);
   free(id->created_on);
   free(id->modified_by);
   free(id->last_modified);
   free(id);
}

/* i/o */

void save_id_handler( ID_HANDLER *handler )
{
   FILE *fp;
   ITERATOR Iter;
   I_ID *id;
   char location[MAX_BUFFER];

   mud_printf( location, "../system/%s.handler", id_handler_names[handler->type] );
   if( ( fp = fopen( location, "w" ) ) == NULL )
   {
      bug( "%s: unable to open file to write: %s", __FUNCTION__, location );
      return;
   }

   fwrite_id_handler( handler, fp );

   AttachIterator( &Iter, handler->free_ids );
   while( ( id = (I_ID *)NextInList(&Iter) ) != NULL )
      fwrite_i_id( id, fp );
   DetachIterator( &Iter );

   fprintf( fp, "%s\n", FILE_TERMINATOR );
   fclose( fp );
   return;
}

bool load_id_handler( ID_HANDLER *handler )
{
   FILE *fp;
   I_ID *id;
   char *word;
   char location[MAX_BUFFER];
   bool found, done = FALSE;

   mud_printf( location, "../handlers/%s.handler", id_handler_names[handler->type] );
   if( ( fp = fopen( location, "r" ) ) == NULL )
   {
      bug( "%s: unable to open file to read: %s", __FUNCTION__, location );
      return FALSE;
   }

   word = ( feof( fp ) ? FILE_TERMINATOR : fread_word( fp ) );

   if( strcmp( word, "#IDHANDLER" ) )
   {
      bug( "%s: %s not started with proper tag.", __FUNCTION__, location );
      return FALSE;
   }

   while( !done )
   {
      found = FALSE;
      switch( word[1] )
      {
         case 'O':
            if( !strcasecmp( word, "EOF" ) ) {done = TRUE; found = TRUE; break; }
            break;
         case 'I':
            if( !strcmp( word, "#IDHANDLER" ) )
            {
               found = TRUE;
               fread_id_handler( handler, fp );
               break;
            }
            if( !strcmp( word, "#I_ID" ) )
            {
               found = TRUE;
               id = fread_i_id( fp );
               AttachToList( id, handler->free_ids );
               break;
            }
            break;
      }
      if( !found )
      {
         bug( "%s: word key not known, %s", __FUNCTION__, word );
         return FALSE;
      }
      if( !done )
         word = ( feof( fp ) ? FILE_TERMINATOR : fread_word( fp ) );
   }
   fclose( fp );
   return TRUE;
}

void fwrite_id_handler( ID_HANDLER *handler, FILE *fp )
{
   fprintf( fp, "#IDHANDLER\n" );
   fprintf( fp, "TopID      %d\n", handler->top_id );
   fprintf( fp, "#END\n" );
   return;
}

void fread_id_handler( ID_HANDLER *handler, FILE *fp )
{
   char *word;
   bool found, done = FALSE;

   word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   while( !done )
   {
      found = FALSE;
      switch( word[0] )
      {
         case '#':
            if( !strcasecmp( word, "#END" ) ) { done = TRUE; found = TRUE; break; }
         case 'T':
            IREAD( "TopID", handler->top_id );
            break;
      }
      if( !found )
      {
         bug( "%s: bad file format %s.", __FUNCTION__, word );
         return;
      }
      if( !done )
         word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   }
   return;
}

void fwrite_i_id( I_ID *id, FILE *fp )
{
   fprintf( fp, "#I_ID\n" );
   fprintf( fp, "ID           %d\n", id->id );
   fprintf( fp, "CreatedBy    %s~\n", id->created_by );
   fprintf( fp, "CreatedOn    %s~\n", id->created_on );
   fprintf( fp, "ModifiedBy   %s~\n", id->modified_by );
   fprintf( fp, "ModifiedLast %s~\n", id->last_modified );
   fprintf( fp, "#END\n" );
   return;

}

I_ID *fread_i_id( FILE *fp )
{
   I_ID *id = NULL;
   char *word;
   bool found, done = FALSE;

   CREATE( id, I_ID, 1 );
   word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   while( ! done )
   {
      found = FALSE;
      switch( word[0] )
      {
         case '#':
            if( !strcasecmp( word, "#END" ) ){done = TRUE; found = TRUE; break;}
         case 'C':
            SREAD( "CreatedBy", id->created_by );
            SREAD( "CreatedOn", id->created_on );
            break;
         case 'I':
            IREAD( "ID", id->id );
            break;
         case 'M':
            SREAD( "ModifiedBy", id->modified_by );
            SREAD( "ModifiedLast", id->last_modified );
            break;
      }
      if( !found )
      {
         bug( "%s: bad file format %s.", __FUNCTION__, word );
         free_i_id( id );
         return NULL;
      }
      if( !done )
         word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   }
   return id;
}

/* utility */
ID_HANDLER *get_id_handler( int type )
{
   ID_HANDLER *handler = NULL;
   ITERATOR Iter;

   AttachIterator( &Iter, id_handlers );
   while( ( handler = (ID_HANDLER *)NextInList( &Iter ) ) != NULL )
      if( handler->type == type )
         break;
   DetachIterator( &Iter );
   return handler;
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
   return id;
}

int use_top_id( ID_HANDLER *handler )
{
   handler->top_id++;
   save_id_handler( handler );
   return handler->top_id;
}

int get_top_id( ID_HANDLER *handler )
{
   return handler->top_id;
}

void update_id( D_MOBILE *dMob, I_ID *id )
{
   id->last_modified = ctime( &current_time );
   id->modified_by = strdup( dMob->name );
   return;
}
