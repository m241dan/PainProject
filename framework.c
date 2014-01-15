/* File framework.c
   All methods pertaining to framework.c go here
   Written by Davenge */


/*******************
 * Utility Methods *
 *******************/

/* Methods Pertaining to IDs */

FRAME_ID *create_fid( int id, int create, int modify )
{
   FRAME_ID *fid;

   CREATE( fid, FRAME_ID, 1 );
   fid->id = id;
   fid->created_on = create;
   fid->last_modified = modify;
   return fid;
}

void init_id_handler( void )
{
   CREATE( global_id, ID_HANDLER, 1 );
   global_id->free_ids = AllocList();
   global_id->top_id = 0;
   return;
}

void fwrite_id_handler( void )
{
   FILE *fp;
   FRAME_ID *fid;
   ITERATOR Iter;

   if( ( fp = fopen( "../system/id_handler.dat", "w" ) ) == NULL )
   {
      bug( %s: can't open file to write handler stuff.", __FUNCTION__ );
      return;
   }

   fprintf( fp, "TopID             %d\n", global_id->top_id )
   AttachIterator( &Iter, global_id->free_ids );
   while( ( fid = (FRAME_ID *)NextInList( &Iter ) ) != NULL )
      fprintf( fp, "FreeID      %d %d %d\n", fid->id, fid->created_on, fid->last_modified );
   fprintf( fp, "%s\n", FILE_TERMINATOR );
   fclose( fp );
   return;
}

void fread_id_handler( void )
{
   FRAME_ID *fid;
   FILE *fp;
   bool done = FALSE, found;
   char *word;

   init_id_handler();

   if( ( fp = fopen( "../system/id_handler.dat", "r" ) ) == NULL )
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
               fid = create_fid( fread_number( fp ), fread_number( fp ), fread_number( fp ) );
               AttachToList( 
            }
         case 'T':
      }
   }
}
