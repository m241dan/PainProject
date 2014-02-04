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
   dMob->events = AllocList();
   dMob->commands = AllocList();
   return dMob;
}

void clear_mobile( D_MOBILE *dMob )
{
   dMob->socket = NULL;
   dMob->name = NULL;
   dMob->password = NULL;
   dMob->level = 0;
   dMob->race = 0;
   dMob->account = NULL;
   dMob->ent_wrapper = NULL;
   dMob->at_coord = NULL;
   dMob->workspace = NULL;
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
   clear_mobile_event_list( dMob );
   FreeList( dMob->events );

   clear_mobile_command_list( dMob );
   FreeList( dMob->commands );

   dMob->socket = NULL;

   if( dMob->name )
      free( dMob->name );
   if( dMob->password )
      free( dMob->password );
   dMob->account = NULL;
   dMob->ent_wrapper = NULL; /* this needs work */
   dMob->at_coord = NULL;
   dMob->workspace = NULL; /* this needs work */
   free( dMob );
}

void clear_mobile_command_list( D_MOBILE *dMob )
{
   COMMAND *com;
   ITERATOR Iter;

   if( !dMob->commands )
      return;

   AttachIterator(&Iter, dMob->commands );
   while( ( com = (COMMAND *)NextInList(&Iter) ) != NULL )
   {
      DetachFromList( com, dMob->commands );
      free_command( com );
   }
   DetachIterator(&Iter);
   return;
}

void clear_mobile_event_list( D_MOBILE *dMob )
{
   EVENT_DATA *pEvent;
   ITERATOR Iter;

   if( !dMob->events )
      return;

   AttachIterator(&Iter, dMob->events);
   while ((pEvent = (EVENT_DATA *) NextInList(&Iter)) != NULL)
     dequeue_event(pEvent);
   DetachIterator(&Iter);
   return;
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
         bug( "%s: attempting to save unknown level type.", __FUNCTION__ );
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
         free_mobile( dMob );
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
   fprintf( fp, "Password      %s~\n", dMob->password );
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

void load_mobile_commands( D_MOBILE *dMob )
{
   COMMAND *com;
   int x;

   clear_mobile_command_list( dMob ); /* clear it out before we lost new commands */

   for( x = 0; tabCmd[x].cmd_name[0] != '\0'; x++ ) /* load the new commands that fit our criteria */
      if( tabCmd[x].state == STATE_PLAYING && tabCmd[x].level <= 2 )
      {
         com = copy_command( tabCmd[x] );
         AttachToList( com, dMob->commands );
      }
   return;
}

void char_to_game( D_SOCKET *dsock, D_MOBILE *dMob )
{
   dsock->bust_prompt = TRUE;
   AttachToList( dMob, dmobile_list );
   text_to_buffer( dsock, "You enter the Mud.\r\n" );

   /* a temporary hack */
   if( !check_coord( 0, 0, 0 ) )
      create_coord( 0, 0, 0 );

   mob_to_coord( dMob, get_coord( 0, 0, 0 ) );
   return;
}

void mob_from_coord( D_MOBILE *dMob )
{
   DetachFromList( dMob->ent_wrapper, dMob->at_coord->entities );
   return;
}

void mob_to_coord( D_MOBILE *dMob, COORD *coordinate)
{
   if( !coordinate )
   {
      bug( "%s: attempting to move %s to a NULL coordinate", __FUNCTION__, dMob->name );
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


   if( !dMob->at_coord->exits[dir] )
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
   mob_to_coord( dMob, dMob->at_coord->exits[dir] );
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
      if( dMob->at_coord->exits[x] )
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
   LIST *flags = pull_flags( arg );

   if( dMob->workspace )
   {
   }
}

void cmd_create_framework( void *passed, char *arg )
{
   D_MOBILE *dMob = (D_MOBILE *)passed;
   FRAMEWORK *framework;
   char arg2[MAX_BUFFER];
   int type;

   arg = one_arg( arg, arg2 );

   if( check_work( dMob ) )
      return;

   if( ( type = match_string_table( arg2, framework_names ) ) == -1 )
   {
      text_to_mobile( dMob, "Invalid framework type.\r\n" );
      return;
   }

   if( ( framework = create_framework( dMob, type ) ) == NULL )
   {
      bug( "%s: something has really fucked up here.", __FUNCTION__ );
      return;
   }

   add_frame_to_workspace( framework, dMob );
   text_to_mobile( dMob, "A new room framework has been added to your workspace.\r\n" );
   return;
}
