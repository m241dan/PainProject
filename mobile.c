/* File mobile.c
   All methods pertaining to the mobile go here
   Written by Davenge */

#include <ctype.h>
#include "mud.h"

/*******************
 * Utility Methods *
 *******************/

/* creation */

D_MOBILE *init_mobile( void )
{
   D_MOBILE *dMob;

   CREATE( dMob, D_MOBILE, 1 );
   clear_mobile( dMob );
   return dMob;
}

void clear_mobile( D_MOBILE *dMob )
{
   dMob->socket = NULL;
   dMob->name = NULL;
   dMob->password = NULL;
   dMob->level = LEVEL_NPC;
   dMob->race = 0;
   dMob->account = NULL;
   dMob->ent_wrapper = NULL;
   dMob->workspace = NULL;
   dMob->editing = NULL;
   return;
}

/* deletion */
void unload_mobile( D_MOBILE *dMob )
{
   DetachFromList( dMob, dmobile_list );
   free_mobile( dMob );
   return;
}

void free_mobile( D_MOBILE *dMob )
{
   clear_mobile_command_list( dMob );
   FreeList( dMob->commands );

   clear_mobile_event_list( dMob );
   FreeList( dMob->events );

   dMob->socket = NULL;

   if( dMob->name )
      free( dMob->name );
   if( dMob->password )
      free( dMob->password );
   dMob->account = NULL;

   if( dMob->ent_wrapper )
      free_entity( dMob->ent_wrapper );
   dMob->ent_wrapper = NULL;

   if( dMob->workspace )
      unset_mobile_workspace( dMob );
   dMob->editing = NULL;
   free( dMob );
}

void save_mobile( D_MOBILE *dMob )
{
   FILE *fp;
   char location[MAX_BUFFER];

   if( !valid_mobile( dMob ) )
      return;

   switch( dMob->level )
   {
      default:
         bug( "%s: attempting to save unknown level '%d'.", __FUNCTION__, dMob->level );
         return;
      case LEVEL_NPC:
         mud_printf( location, "../instances/mobiles/npcs/%s%d.instance", capitalize( dMob->name ), dMob->id->id );
         break;
      case LEVEL_PLAYER:
         mud_printf( location, "../instances/mobiles/players/%s.pfile", capitalize( dMob->name ) );
         break;
      case LEVEL_ADMIN:
         mud_printf( location, "../instances/mobiles/admins/%s.pfile", capitalize( dMob->name ) );
         break;
      case LEVEL_GOD:
         mud_printf( location, "../instances/mobiles/gods/%s.pfile", capitalize( dMob->name ) );
         break;
   }

   if( ( fp = fopen( location, "w" ) ) == NULL )
   {
      bug( "%s: cannot open %s to write.", __FUNCTION__, location );
      return;
   }

   fwrite_mobile( dMob, fp );
   fwrite_entity_data( dMob->ent_wrapper, fp );
   fprintf( fp, "%s", FILE_TERMINATOR );
   fclose( fp );
   return;
}

bool load_mobile( const char *location, D_MOBILE *dMob )
{
   FILE *fp;
   char *word;
   bool found, done = FALSE;

   if( ( fp = fopen( location, "r" ) ) == NULL )
      return FALSE;

   word = ( feof( fp ) ? FILE_TERMINATOR : fread_word( fp ) );

   if( strcmp( word, "#MOBILE" ) )
   {
      bug( "%s: attempting to read a mobile file that is not tagged as such.", __FUNCTION__ );
      fclose( fp );
      return FALSE;
   }

   while( !done )
   {
      switch( word[1] )
      {
         case 'O':
            if( !strcasecmp( word, "EOF" ) ) { found = TRUE; done = TRUE; break; }
            break;
         case 'E':
            if( !strcmp( word, "#ENTITY" ) )
            {
               found = TRUE;
               if( ( dMob->ent_wrapper = fread_entity_data( fp ) ) == NULL )
                  found = FALSE;
               break;
            }
            break;
         case 'M':
            if( !strcmp( word, "#MOBILE" ) )
            {
               found = TRUE;
               if( !fread_mobile( dMob, fp ) )
                  found = FALSE;
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

void fwrite_mobile ( D_MOBILE *dMob, FILE *fp )
{
   fprintf( fp, "#MOBILE\n" );
   fprintf( fp, "Name          %s~\n", dMob->name );
   if( dMob->password )
      fprintf( fp, "Password      %s~\n", dMob->password );
   fprintf( fp, "Account       %s~\n", dMob->account->name );
   fprintf( fp, "Level         %d\n", dMob->level );
   fprintf( fp, "Race          %d\n", dMob->race );
   fprintf( fp, "#END\n" );
   return;
}

bool fread_mobile( D_MOBILE *dMob, FILE *fp )
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
            if( !strcasecmp( word, "#END" ) ) { found = TRUE; done = TRUE; break; }
            break;
         case 'A':
            if( !strcmp( word, "Account" ) )
            {
               found = TRUE;
               if( ( dMob->account = get_account_from_name( fread_string( fp ) ) ) == NULL )
               {
                  bug( "%s: can't find account from name.", __FUNCTION__ );
                  found = FALSE;
               }
               break;
            }
            break;
         case 'L':
            IREAD( "Level", dMob->level );
            break;
         case 'N':
            SREAD( "Name", dMob->name );
            break;
         case 'P':
            SREAD( "Password", dMob->password );
            break;
         case 'R':
            IREAD( "Race", dMob->race );
            break;
      }
      if( !found )
      {
         bug( "%s: bad file foramt %s", __FUNCTION__, word );
         return FALSE;
      }
      if( !done )
         word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   }
   return TRUE;
}

/* utility */

void mobile_prompt( D_SOCKET *dsock )
{
   BUFFER *buf;
   D_MOBILE *dMob;

   if( ( dMob = dsock->player ) == NULL )
   {
      bug( "%s: socket has no player to send prompt to.", __FUNCTION__ );
      return;
   }

   buf = buffer_new( MAX_BUFFER );

   if( dMob->workspace && dMob->workspace->name && dMob->workspace->name[0] != '\0' && dsock->account)
   {
      FRAMEWORK *fWork;
      ITERATOR Iter;
      int width = dsock->account->pagewidth;

      bprintf( buf, "/%s\\\r\n", print_header( dMob->workspace->name, "-", ( width - 2 )  ) );

      AttachIterator( &Iter, dMob->workspace->contents );
      while( ( fWork = (FRAMEWORK *)NextInList( &Iter ) ) != NULL )
         bprintf( buf, "| %-*.*s |\r\n", ( width - 4 ), ( width - 4 ), fWork->name );
      DetachIterator( &Iter );
      bprintf( buf, "\\%s/\r\n", print_header( "End Workspace", "-", ( width - 2 ) ) );
   }

   bprintf( buf, "Davengine's NULL Prompt:> " );
   text_to_buffer( dsock, buf->data );
   buffer_free( buf );
   return;
}

/* Movement */
bool char_to_game( D_MOBILE *dMob )
{
   dMob->socket->bust_prompt = TRUE;
   AttachToList( dMob, dmobile_list );
   text_to_mobile( dMob, "You enter the Mud.\r\n" );

   /* a temporary hack */
   if( !check_coord( 0, 0, 0 ) )
      create_coord( 0, 0, 0 );

   mob_to_coord( dMob, get_coord( 0, 0, 0 ) );
   return TRUE;
}

void mob_from_coord( D_MOBILE *dMob )
{
   return;
}

void mob_to_coord( D_MOBILE *dMob, COORD *coordinate)
{
   if( !coordinate )
   {
      bug( "%s: attempting to move %s to a NULL coordinate", __FUNCTION__, dMob->name );
      return;
   }
   if( !dMob->ent_wrapper )
   {
      bug( "%s: %s does not have an entity wrapper.", __FUNCTION__, dMob->name );
      return;
   }
   dMob->at_coord = coordinate;
   AttachToList( dMob->ent_wrapper, coordinate->entities );
   return;
}

void move_char( D_MOBILE *dMob, int dir )
{
   BUFFER *buf = buffer_new( MAX_BUFFER );
   COORD *prev_coord = dMob->at_coord;
   ITERATOR Iter;
   D_MOBILE *r_dMob;
   int x = prev_coord->pos_x, y = prev_coord->pos_y, z = prev_coord->pos_z;

   /* Set X, Y, Z to the destinations variables */

   switch( dir )
   {
      case DIR_NORTH:
         y++;
         break;
      case DIR_EAST:
         x++;
         break;
      case DIR_SOUTH:
         y--;
         break;
      case DIR_WEST:
         x--;
         break;
      case DIR_UP:
         z++;
         break;
      case DIR_DOWN:
         z--;
         break;
   }


   if( !dMob->at_coord->connected[dir] )
   {
      if( !IS_ADMIN( dMob ) )
      {
         text_to_mobile( dMob, "You cannot move that way.\r\n" );
         return;
      }
      bprintf( buf, "You wave your hand and create a new coordinate to the %s.\r\n", exit_directions[dir] );
      text_to_mobile( dMob, buf->data );
      buffer_clear( buf );
      bprintf( buf, "%s waves their hand and create a coordinate to the %s.\r\n", dMob->name, exit_directions[dir] );
      AttachIterator( &Iter, prev_coord->entities );
      while( ( r_dMob = (D_MOBILE *)NextInList( &Iter ) ) != NULL )
      {
         if( r_dMob == dMob )
            continue;
         text_to_mobile( r_dMob, buf->data );
      }
      DetachIterator( &Iter );
      buffer_clear( buf );
      create_coord( x, y, z );
   }
   mob_from_coord( dMob );
   mob_to_coord( dMob, dMob->at_coord->connected[dir] );
   bprintf( buf, "You move %s.\r\n", exit_directions[dir] );
   text_to_mobile( dMob, buf->data );
   buffer_clear( buf );
   bprintf( buf, "%s moves %s.\r\n", dMob->name, exit_directions[dir] );
   cmd_look( dMob, "" );
   AttachIterator( &Iter, prev_coord->entities );
   while( ( r_dMob = (D_MOBILE *)NextInList( &Iter ) ) != NULL )
   {
      if( r_dMob == dMob )
         continue;
      text_to_mobile( r_dMob, buf->data );
   }
   DetachIterator( &Iter );
   buffer_free( buf );
   return;
}

/**************************
 * Mobile Command Methods *
 **************************/

void cmd_look( void *passed, char *arg )
{
   BUFFER *buf = buffer_new( MAX_BUFFER );
   D_MOBILE *dMob = (D_MOBILE *)passed;
   ENTITY *ent;
   D_MOBILE *r_dMob;
   ITERATOR Iter;
   int x;

   bprintf( buf, "You are at coordinate (X): %d (Y): %d (Z): %d\r\n", dMob->at_coord->pos_x, dMob->at_coord->pos_y, dMob->at_coord->pos_z );

   bprintf( buf, "----------\r\nExits    |\r\n----------\r\n" );

   for( x = 0; x < MAX_DIRECTION; x++ )
      if( dMob->at_coord->connected[x] )
         bprintf( buf, "%-10.10s-\r\n", exit_directions[x] );
   AttachIterator( &Iter, dMob->at_coord->entities );
   while( ( ent = (ENTITY *)NextInList( &Iter ) ) != NULL )
   {
      switch( ent->type )
      {
         case MOBILE_ENTITY:
            r_dMob = (D_MOBILE *)ent->content;
            if( r_dMob == dMob )
               continue;
            bprintf( buf, "%s the playah.\r\n", r_dMob->name );
            break;
      }
   }
   DetachIterator( &Iter );
   text_to_mobile( dMob, buf->data );
   buffer_free( buf );
}

void cmd_north( void *passed, char *arg )
{
   move_char( (D_MOBILE *)passed, DIR_NORTH );
}

void cmd_east( void *passed, char *arg )
{
   move_char( (D_MOBILE *)passed, DIR_EAST );
}

void cmd_south( void *passed, char *arg )
{
   move_char( (D_MOBILE *)passed, DIR_SOUTH );
}

void cmd_west( void *passed, char *arg )
{
   move_char( (D_MOBILE *)passed, DIR_WEST );
}

void cmd_up( void *passed, char *arg )
{
   move_char( (D_MOBILE *)passed, DIR_UP );
}

void cmd_down( void *passed, char *arg )
{
   move_char( (D_MOBILE *)passed, DIR_DOWN );
}


/****************
 * OLC Commands *
 ****************/

void cmd_open_workspace( void *passed, char *arg )
{
   D_MOBILE *dMob = (D_MOBILE *)passed;
   LIST *flags = AllocList();
   WORKSPACE *wSpace;
   char no_flags_buf[MAX_BUFFER];
   char buf[MAX_BUFFER];
   bool creator = FALSE;

   memset( &no_flags_buf[0], 0, sizeof( no_flags_buf ) );

   if( dMob->workspace )
   {
      mob_printf( dMob, "You already have a workspace opened: %s\r\n", dMob->workspace->name );
      return;
   }

   if( !arg || arg[0] == '\0' )
   {
      mob_printf( dMob, "Proper Usage: w_open <name>\r\n - valid flags: -new\r\n" );
      return;
   }

   pull_flags( flags, arg, no_flags_buf );
   one_arg( no_flags_buf, buf );

   if( ( wSpace = get_workspace_from_list( buf ) ) == NULL )
   {
      if( get_flag( flags, "-new" ) )
      {
         wSpace = init_workspace();
         if( !create_workspace( dMob, wSpace, buf ) )
         {
            mob_printf( dMob, "You could not create the workspace titled: %s\r\n", buf );
            free_workspace( wSpace );
            free_flag_list( flags );
            return;
         }
         else
         {
            mob_printf( dMob, "Workspace '%s': Created on %s.\r\n", buf, ctime( &current_time ) );
            save_workspace( wSpace );
         }
      }
      else
      {
         mob_printf( dMob, "No such workspace exists.\r\n" );
         free_flag_list( flags );
         return;
      }
   }

   free_flag_list( flags );

   if( !strcmp( dMob->name, wSpace->id->created_by ) )
      creator = TRUE;

   if( wSpace->type != WORKSPACE_PUBLIC && !creator )
   {
      mob_printf( dMob, "You don't have permission to open %s.\r\n", wSpace->name );
      return;
   }

   if( wSpace->who_using && wSpace->who_using != dMob )
   {
      mob_printf( dMob, "%s is already using that workspace.\r\n", wSpace->who_using ? wSpace->who_using->name : "(null)" );
      return;
   }

   mob_printf( dMob, "You open %s workspace.\r\n", wSpace->name );
   set_mobile_workspace( dMob, wSpace );
   return;
}

void cmd_close_workspace( void *passed, char *arg )
{
   D_MOBILE *dMob = (D_MOBILE *)passed;

   if( !dMob->workspace )
   {
      mob_printf( dMob, "You do not have a work space open to close.\r\n" );
      return;
   }
   save_workspace( dMob->workspace );
   unset_mobile_workspace( dMob );
   mob_printf( dMob, "Workspace closed.\r\n" );
   return;
}


void cmd_create_framework( void *passed, char *arg )
{
   D_MOBILE *dMob = (D_MOBILE *)passed;
   FRAMEWORK *framework;
   char arg1[MAX_BUFFER];
   int type;

   if( !arg || arg[0] == '\0' )
   {
      mob_printf( dMob, "Proper Usage: create <type> <name>" );
      return;
   }

   if( !check_work( dMob ) )
      return;

   arg = one_arg( arg, arg1 );

   if( arg1[0] == '\0' )
   {
      mob_printf( dMob, "You need to input a type.\r\n Types: %s\r\n", get_table( framework_names ) );
      return;
   }

   if( ( type = match_string_table( arg1, framework_names ) ) == -1 )
   {
      mob_printf( dMob, "Invalid framework type.\r\n Types: %s", get_table( framework_names ) );
      return;
   }

   if( arg[0] == '\0' )
   {
      mob_printf( dMob, "You need to name your framework.\r\n" );
      return;
   }

   if( check_frame_name( arg ) )
   {
      mob_printf( dMob, "A framework with that name already exists.\r\n" );
      return;
   }


   if( ( framework = create_framework( dMob, type ) ) == NULL )
   {
      bug( "%s: something has really fucked up here.", __FUNCTION__ );
      return;
   }

   set_framework( framework, (VALUE)arg, FRAME_NAME );
   add_frame_to_workspace( framework, dMob );
   text_to_mobile( dMob, "A new room framework has been added to your workspace.\r\n" );
   return;
}

void cmd_pagewidth( void *passed, char *arg )
{
   D_MOBILE *dMob = (D_MOBILE *)passed;
   int width;

   if( !dMob )
   {
      bug( "%s: passed a NULL dMob.", __FUNCTION__ );
      return;
   }
   if( !dMob->account )
   {
      bug( "%s: dMob %s has NULL account.", __FUNCTION__, dMob->name );
      return;
   }
   if( !arg || arg[0] == '\0' )
   {
      mob_printf( dMob, "Proper usage: pagewidth #\r\n" );
      return;
   }
   if( !is_number( arg ) )
   {
      mob_printf( dMob, "You must put in a number.\r\n" );
      return;
   }

   width = atoi( arg );
   set_account( dMob->account, width, ACT_PAGEWIDTH );
   mob_printf( dMob, "Width set to %d.\r\n", width );
   return;
}
