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
   dMob->passowrd = NULL;
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

   swtich( dMob->level )
   {
      default:
         bug( "%s: attempting to save unknown level type.", __FUNCTION__ );
         return;
      case LEVEL_NPC:
         log_string( "%s: not ready to save NPCs yet.", __FUNCTION__ );
         return;
      case LEVEL_PLAYER:
      case LEVEL_ADMIN:
      case LEVEL_GOD:
         mud_printf( location, "../players/%s.pfile", capitalize( dMob->name ) );
         break;
   }
}


/* Method to save any mobile to its proper place */

void save_mobile( D_MOBILE *dMob )
{
   FILE *fp;
   char location[MAX_BUFFER];

   if( !valid_mobile( dMob ) )
      return;

   switch( dMob->level )
   {
      default:
         text_to_mobile( dMob, "Unknown mobile level, cannot save." );
         bug( "%s: %s attempting to save with %d level.", __FUNCTION__, dMob->name, dMob->level );
         return;
      case LEVEL_GOD:
      case LEVEL_ADMIN:
      case LEVEL_PLAYER:
         mud_printf( location, "../accounts/%s/%s.pfile", capitalize( dMob->account->name ), capitalize( dMob->name ) );
         if( ( fp = fopen( location, "w" ) ) == NULL )
         {
            bug( "%s: Unable to write account data to %s's pfile.", __FUNCTION__, dMob->name );
            return;
         }
         fwrite_account_data( fp, dMob );
         fprintf( fp, "%s\n", FILE_TERMINATOR );
         fclose( fp );
         if( dMob->loaded )
         {
            mud_printf( location, "../accounts/%s/%s.gfile", capitalize( dMob->account->name ), capitalize( dMob->name ) );
            if( ( fp = fopen( location, "w" ) ) == NULL )
            {
               bug( "%s: Unable to write game data to %s's gfile.", __FUNCTION__, dMob->name );
               return;
            }
            fwrite_game_data( fp, dMob );
         }
         break;
   }
   fprintf( fp, "%s\n", FILE_TERMINATOR );
   fclose( fp );
   return;
}

void fwrite_account_data( FILE *fp, D_MOBILE *dMob )
{
   fprintf( fp, "#ACCOUNT\n" );
   fprintf( fp, "Name            %s~\n", dMob->name);
   fprintf( fp, "Level           %d\n",  dMob->level);
   fprintf( fp, "Password        %s~\n", dMob->password);
   fprintf( fp, "Race            %d\n",  dMob->race);
   fprintf( fp, "#END\n" );
   return;
}

void fwrite_game_data( FILE *fp, D_MOBILE *dMob )
{
   fprintf( fp, "#PLAYER\n" );
   fprintf( fp, "#END\n" );
   return;
}

/* unload a mobile, with options */

void unload_mobile( D_MOBILE *dMob, bool partial )
{
   if( !dMob->loaded && partial ) /* a character already logged out and want a partial unload, do nothing */
      return;
   else if( dMob->loaded && partial ) /* logging out of a character to an account */
      free_mobile_game_data( dMob );
   else if( dMob->loaded && !partial ) /* character logged in and we want to completely quit the game */
   {
      free_mobile_game_data( dMob );
      free_mobile_account_data( dMob );
      PushStack( dMob, dmobile_free );
      return;
   }
   else if( !dMob->loaded && !partial ) /* this would be quitting the game from an account and freeing up the LIST *character in account structure */
   {
      free_mobile_account_data( dMob );
      PushStack( dMob, dmobile_free );
      return;
   }
   return;
}

void alloc_mobile_lists( D_MOBILE *dMob ) /* alloc the dMobiles lists */
{
   dMob->commands = AllocList();
   dMob->events = AllocList();
   return;
}

void free_mobile_lists( D_MOBILE *dMob ) /* deallocate the dMobiles list memory */
{
   if( dMob->commands )
      FreeList( dMob->commands );
   if( dMob->events )
      FreeList( dMob->events );
   return;
}

/* Free all the data related to playing the game only */
void free_mobile_game_data(D_MOBILE *dMob)
{
   if( dMob->loaded )
   {
      DetachFromList(dMob, dmobile_list);

      if (dMob->socket) dMob->socket->player = NULL;

      /* Free Up Event List */
      clear_mobile_event_list( dMob );
      /* Free up Comand List */
      clear_mobile_command_list( dMob );
      /* Now for the List's Memory */
      free_mobile_lists( dMob );
   }
   return;
}

/* Free all dMob account data */
void free_mobile_account_data( D_MOBILE *dMob )
{
   free( dMob->name );
   free( dMob->password );
   return;
}

void clear_mobile( D_MOBILE *dMob )
{
   dMob->name         =  NULL;
   dMob->password     =  NULL;
   dMob->level        =  LEVEL_PLAYER;
   dMob->race         =  RACE_HUMAN;

   if( dMob->loaded )
      alloc_mobile_lists( dMob );
}

void load_mobile( ACCOUNT *account, char *player, bool partial, D_MOBILE *dMob )
{
   FILE *fp;
   char *word;
   bool found, done = FALSE;
   char pFile[MAX_BUFFER];
   char gFile[MAX_BUFFER];

   if( account ) /* if its a player */
   {
      mud_printf( pFile, "../accounts/%s/%s.pfile", capitalize( account->name ), capitalize( player ) );
      if( ( fp = fopen( pFile, "r" ) ) == NULL )
      {
         bug( "%s: Unable to read account data %s.", __FUNCTION__, player );
         return;
      }
      word = fread_word( fp );
      while( !done )
      {
         found = FALSE;
         switch( word[0] )
         {
            case '#':
               if( !strcmp( word, "#ACCOUNT" ) )
               {
                  found = TRUE;
                  fread_mobile_account_data( fp, dMob );
               }
               break;
            case 'E':
               if (!strcasecmp(word, "EOF")) {done = TRUE; found = TRUE; break;}
               break;
         }
         if( !found )
         {
            bug( "%s: unexpected '%s' in %s.", __FUNCTION__, word, pFile );
            unload_mobile( dMob, FALSE );
            dMob = NULL;
            return;
         }
         /* read one more */
         if( !done )
            word = fread_word(fp);
      }
      if( !partial )
      {
         fclose( fp );
         mud_printf( gFile, "../accounts/%s/%s.gfile", capitalize( account->name), capitalize( player ) );
         if( ( fp = fopen( gFile, "r" ) ) == NULL )
         {
            bug( "%s: Unable to read game data %s.", __FUNCTION__, player );
            return;
         }
         word = fread_word( fp );
         while( !done )
         {
            found = FALSE;
            switch( word[0] )
            {
               case '#':
                  if( !strcmp( word, "#PLAYER" ) )
                  {
                     found = TRUE;
                     fread_mobile_game_data( fp, dMob );
                  }
                  break;
               case 'E':
                  if (!strcasecmp(word, "EOF")) {done = TRUE; found = TRUE; break;}
                  break;
            }
            if( !found )
            {
               bug( "%s: unexpected '%s' in %s.", __FUNCTION__, word, gFile );
               unload_mobile( dMob, FALSE );
               dMob = NULL;
               return;
            }
            /* read one more */
            if( !done )
               word = fread_word(fp);
         }
      }
   }
   else /* if its an npc */
   {
      return; /* nothing yet */
   }
   fclose( fp );
   return;
}

void fread_mobile_account_data( FILE *fp, D_MOBILE *dMob )
{
   char *word;
   bool found, done = FALSE;


   if( !dMob )
   {
     if( StackSize( dmobile_free ) <= 0 )
        CREATE( dMob, D_MOBILE, 1 );
     else
       dMob = (D_MOBILE *)PopStack( dmobile_free );
   }
  /* load data */
  word = fread_word(fp);
  while (!done)
  {
    found = FALSE;
    switch (word[0])
    {
      case '#':
        if( !strcmp( word, "#END" ) ) { done = TRUE; found = TRUE; break; }
        break;
      case 'L':
        IREAD( "Level",     dMob->level     );
        break;
      case 'N':
        SREAD( "Name",      dMob->name      );
        break;
      case 'P':
        SREAD( "Password",  dMob->password  );
        break;
      case 'R':
        IREAD( "Race", dMob->race );
        break;
    }
    if (!found)
    {
      bug("%s: unexpected '%s' in %s.", __FUNCTION__, word, dMob->name);
      unload_mobile( dMob, FALSE );
      dMob = NULL;
      return;
    }

    /* read one more */
    if (!done) word = fread_word(fp);
  }
  return;
}

void fread_mobile_game_data( FILE *fp, D_MOBILE *dMob )
{
   char *word;
   bool found, done = FALSE;

   alloc_mobile_lists( dMob );

   if( !dMob ) /* don't relaly need this but decided to keep it there JUST in case I ever want to just load game data */
   {
      if( StackSize( dmobile_free ) <= 0 )
         CREATE( dMob, D_MOBILE, 1 );
      else
         dMob = (D_MOBILE *)PopStack( dmobile_free );
   }

   word = fread_word( fp );
   while( !done )
   {
      found - FALSE;
      switch( word[0] )
      {
         case '#':
            if( !strcmp( word, "#END" ) ) { done = TRUE; found = TRUE; break; }
            break;
      }
      if( !found )
      {
         bug( "%s: unexpected '%s' in %s.", __FUNCTION__, word, dMob->name );
         unload_mobile( dMob, FALSE );
         return;
      }
      if( !done ) word = fread_word( fp );
   }
   return;
}


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
   dsock->player = dMob;
   dMob->socket = dsock;
   dMob->loaded = TRUE;
   load_mobile( dMob->account, dMob->name, FALSE, dMob );
   wrap_entity( dMob, MOBILE_ENTITY );
   change_socket_state( dsock, STATE_PLAYING );
   dsock->bust_prompt = TRUE;
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
