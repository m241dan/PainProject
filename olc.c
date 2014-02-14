/* File olc.c
   All methods pertaining to the olc go here
   Written by Davenge */

#include "mud.h"

/*******************
 * Utility Methods *
 *******************/

/* creation */
WORKSPACE *init_workspace( void )
{
   WORKSPACE *wSpace;

   CREATE( wSpace, WORKSPACE, 1 );
   wSpace->contents = AllocList();
   wSpace->type = WORKSPACE_PRIVATE;
   return wSpace;
}

bool create_workspace( D_MOBILE *dMob, WORKSPACE *wSpace, const char *name )
{
   if( !dMob )
   {
      bug( "%s: called with NULL dMob.", __FUNCTION__ );
      return FALSE;
   }
   if( !wSpace )
   {
      bug( "%s: called with a NULL wSpace by %s.", __FUNCTION__, dMob->name );
      return FALSE;
   }
   if( !name || name[0] == '\0' )
   {
      bug( "%s: called with bad name given.", __FUNCTION__ );
      return FALSE;
   }
   if( get_workspace_from_list( name ) )
   {
      text_to_mobile( dMob, "A workspace with that name already exists.\r\n" );
      return FALSE;
   }
   wSpace->id = create_new_id( dMob, WORKSPACE_STRUCT );
   wSpace->name = strdup( name );
   AttachToList( wSpace, workspaces );
   return TRUE;
}

/* deletion */
void free_workspace( WORKSPACE *wSpace )
{
   FRAMEWORK *frame;
   ITERATOR Iter;

   if( wSpace->who_using )
      unset_mobile_workspace( wSpace->who_using );
   free( wSpace->name );
   if( wSpace->id )
      free_i_id( wSpace->id );
   while( ( frame = (FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      DetachFromList( frame, wSpace->contents );
   DetachIterator( &Iter );
   FreeList( wSpace->contents );
   free( wSpace );
   return;
}

/* i/o */
bool load_workspaces( void )
{
   DIR *directory;
   struct dirent *entry;
   char location[MAX_BUFFER];
   WORKSPACE *wSpace;

   directory = opendir( "../workspaces" );
   for( entry = readdir( directory ); entry; entry = readdir( directory ) )
   {
      if( !string_contains( entry->d_name, ".workspace" ) )
         continue;

      wSpace = init_workspace();

      mud_printf( location, "../workspaces/%s", entry->d_name );
      if( !load_workspace( location, wSpace ) )
      {
         free_workspace( wSpace );
         bug( "%s: could not load workspace from file %s.", __FUNCTION__, location );
         continue;
      }
      AttachToList( wSpace, workspaces );
   }
   return TRUE;
}

void save_workspace( WORKSPACE *wSpace )
{
   FILE *fp;
   char location[MAX_BUFFER];

   mud_printf( location, "../workspaces/%d.workspace", wSpace->id->id );
   if( ( fp = fopen( location, "w" ) ) == NULL )
   {
      bug( "%s: cannot open %s to write.", __FUNCTION__, location );
      return;
   }

   fwrite_workspace( wSpace, fp );
   fwrite_i_id( wSpace->id, fp );

   fprintf( fp, "%s\n", FILE_TERMINATOR );
   fclose( fp );
   return;
}

bool load_workspace( const char *location, WORKSPACE *wSpace )
{
   FILE *fp;
   char *word;
   bool found, done = FALSE;


   if( ( fp = fopen( location, "r" ) ) == NULL )
   {
      bug( "%s: cannot open %s to read from.", __FUNCTION__, location );
      return FALSE;
   }

   word = ( feof( fp ) ? FILE_TERMINATOR : fread_word( fp ) );
   if( strcmp( word, "#WORKSPACE" ) )
   {
      bug( "%s: attempting to read a file that isn't tagged as a workspace.", __FUNCTION__ );
      fclose( fp );
      return FALSE;
   }

   while( !done )
   {
      found - FALSE;
      switch( word[1] )
      {
         case 'O':
            if( !strcasecmp( word, "EOF" ) ) { done = TRUE; found = TRUE; break; }
            break;
         case 'W':
            if( !strcmp( word, "#WORKSPACE" ) )
            {
               found = TRUE;
               fread_workspace( wSpace, fp );
               break;
            }
            break;
         case 'I':
            if( !strcmp( word, "#I_ID" ) )
            {
               found = TRUE;
               wSpace->id = fread_i_id( fp );
               break;
            }
            break;
      }
      if( !found )
      {
         bug( "%s: bad file format %s.", __FUNCTION__, word );
         fclose( fp );
         return FALSE;
      }
      if( !done )
         word = ( feof( fp ) ? FILE_TERMINATOR : fread_word( fp ) );
   }
   fclose( fp );
   return TRUE;
}

void fwrite_workspace( WORKSPACE *wSpace, FILE *fp )
{
   FRAMEWORK *frame;
   ITERATOR Iter;

   fprintf( fp, "#WORKSPACE\n" );
   fprintf( fp, "Name         %s~\n", wSpace->name );
   fprintf( fp, "Type         %d\n", wSpace->type );

   AttachIterator( &Iter, wSpace->contents );
   while( ( frame = (FRAMEWORK *)NextInList( &Iter ) ) != NULL )
      fprintf( fp, "Frame        %d %d\n", frame->type, frame->id->id );
   DetachIterator( &Iter );

   fprintf( fp, "#END\n" );
   return;
}

void fread_workspace( WORKSPACE *wSpace, FILE *fp )
{
   FRAMEWORK *frame;
   char *word;
   bool found, done = FALSE;

   word = ( feof( fp ) ? "#END" : fread_word( fp ) );

   while( !done )
   {
      found = FALSE;
      switch( word[0] )
      {
         case '#':
            if( !strcasecmp( word, "#END" ) ){ done = TRUE; found = TRUE; break; }
            break;
         case 'F':
            if( !strcmp( word, "Frame" ) )
            {
               int x1, x2;
               found = TRUE;
               x1 = fread_number( fp );
               x2 = fread_number( fp );
               frame = get_frame( x1, x2 );
               AttachToList( frame, wSpace->contents );
               break;
            }
            break;
        case 'N':
           SREAD( "Name", wSpace->name );
           break;
        case 'T':
           IREAD( "Type", wSpace->type );
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



/* Utility */
bool check_work( D_MOBILE *dMob )
{
   if( !dMob->workspace )
   {
      text_to_mobile( dMob, "You have no workspace open.\r\n" );
      return FALSE;
   }
   return TRUE;
}

void add_frame_to_workspace( FRAMEWORK *frame, D_MOBILE *dMob )
{
   if( !dMob->workspace || !dMob->workspace->contents )
   {
      bug( "%s: NULL workspace or workspace content for %s.", __FUNCTION__, dMob->name );
      return;
   }
   AttachToList( frame, dMob->workspace->contents );
   save_workspace( dMob->workspace );
   return;
}

void set_mobile_workspace( D_MOBILE *dMob, WORKSPACE *wSpace )
{
   if( !wSpace )
   {
      bug( "%s: wSpace is NULL.", __FUNCTION__ );
      return;
   }
   if( !dMob )
   {
      bug( "%s: dMob is NULL.", __FUNCTION__ );
      return;
   }
   if( wSpace->who_using )
   {
      bug( "%s: wSpace is being used, cannot set to a different mobile.", __FUNCTION__ );
      return;
   }

   dMob->workspace = wSpace;
   wSpace->who_using = dMob;
   return;
}

void unset_mobile_workspace( D_MOBILE *dMob )
{
   if( !dMob )
   {
      bug( "%s: dMob is NULL.", __FUNCTION__ );
      return;
   }
   if( !dMob->workspace )
   {
      bug( "%s: dMob '%s' does not have a wSpace set.", __FUNCTION__, dMob->name );
      return;
   }

   dMob->workspace->who_using = NULL;
   dMob->workspace = NULL;
   return;
}

WORKSPACE *get_workspace_from_list( const char *name )
{
   WORKSPACE *wSpace;
   ITERATOR Iter;

   if( SizeOfList( workspaces ) <= 0 )
      return NULL;

   AttachIterator( &Iter, workspaces );
   while( ( wSpace = (WORKSPACE *)NextInList( &Iter ) ) != NULL )
      if( !strcmp( wSpace->name, name ) )
         break;
   DetachIterator( &Iter );

   return wSpace;
}
