/* File account.c
   All functions/methods corresponding with accounts
   Written by Davenge */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "mud.h"


/* creation */
ACCOUNT *init_account( void )
{
   ACCOUNT *account;

   CREATE( account, ACCOUNT, 1 );
   clear_account( account );
   account->characters = AllocList();
   account->commands = AllocList();
   return account;
}

void clear_account( ACCOUNT *account )
{
   account->socket = NULL;
   account->name = NULL;
   account->password = NULL;
   account->level = LEVEL_BASIC;
   account->pagewidth = DEFAULT_PAGEWIDTH;
   account->settings_loaded = FALSE;
   return;
}

CHAR_SHEET *init_char_sheet( void )
{
   CHAR_SHEET *cSheet;

   CREATE( cSheet, CHAR_SHEET, 1 );
   clear_char_sheet( cSheet );
   return cSheet;
}

void clear_char_sheet( CHAR_SHEET *cSheet )
{
   if( cSheet->name )
      free( cSheet->name );
   cSheet->race = 0;
   cSheet->level = 1;
   return;
}

CHAR_SHEET *create_char_sheet( D_MOBILE *dMob )
{
   CHAR_SHEET *cSheet = init_char_sheet();

   cSheet->name = strdup( dMob->name );
   cSheet->race = dMob->race;
   cSheet->level = dMob->level;
   return cSheet;
}

/* deletion */
void unload_account( ACCOUNT *account )
{
   DetachFromList( account, account_list );
   free_account( account );
   return;
}

void free_account( ACCOUNT *account )
{
   clear_char_sheet_list( account );
   FreeList( account->characters );

   clear_account_command_list( account );
   FreeList( account->commands );

   if( account->name )
      free( account->name );
   if( account->password )
      free( account->password );
   account->socket = NULL;
   free( account );
   return;
}

void free_character_sheet( CHAR_SHEET *cSheet )
{
   if( cSheet->name )
      free( cSheet->name );
   free( cSheet );
   return;
}

void clear_char_sheet_list( ACCOUNT *account )
{
   ITERATOR Iter;
   CHAR_SHEET *cSheet;

   AttachIterator( &Iter, account->characters );
   while( ( cSheet = (CHAR_SHEET *)NextInList( &Iter ) ) != NULL )
   {
      DetachFromList( cSheet, account->characters );
      free_character_sheet( cSheet );
   }
   DetachIterator( &Iter );

   return;
}

void clear_account_command_list( ACCOUNT *account )
{
   COMMAND *com;
   ITERATOR Iter;

   if( !account->commands )
      return;

   AttachIterator(&Iter, account->commands );
   while( ( com = (COMMAND *)NextInList(&Iter) ) != NULL )
   {
      DetachFromList( com, account->commands );
      free_command( com );
   }
   DetachIterator(&Iter);
   return;
}

/* i/o */
void save_account( ACCOUNT *account )
{
   FILE *fp;
   CHAR_SHEET *cSheet;
   ITERATOR Iter;
   char aDir[MAX_BUFFER], aFile[MAX_BUFFER];

   /* create the "file directory" for the account, so we can check if it exists */
   mud_printf( aDir, "../accounts/%s", capitalize( account->name ) );

   if( opendir( aDir ) == NULL )
   {
      if( ( mkdir( aDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ) ) != 0 ) /* was unsuccessful */
      {
         bug( "Unable to create folder for the new account: %s", account->name );
         return;
      }
   }

   /* create the "file name" for the account */
   mud_printf( aFile, "%s/account.afile", aDir );

   /* open the pointer to the file, if we can't, spit out a bug msg and return */
   if( ( fp = fopen( aFile, "w" ) ) == NULL )
   {
      bug( "Unable to write to %s's account file", account->name );
      return;
   }

   fwrite_account( account, fp );

   AttachIterator( &Iter, account->characters );
   while( ( cSheet = (CHAR_SHEET *)NextInList( &Iter ) ) != NULL )
      fwrite_char_sheet( cSheet, fp );
   DetachIterator( &Iter );

   fprintf( fp, "%s\n", FILE_TERMINATOR );
   fclose( fp );
   return;
}

bool load_account( const char *location, ACCOUNT *account )
{
   FILE *fp;
   CHAR_SHEET *cSheet;
   char *word;
   bool found, done = FALSE;

   if( ( fp = fopen( location, "r" ) ) == NULL )
      return FALSE;

   word = ( feof( fp ) ? FILE_TERMINATOR : fread_word( fp ) );
   if( strcmp( word, "#ACCOUNT" ) )
   {
      bug( "%s: attempting to read an account file that is not tagged as such.", __FUNCTION__ );
      fclose( fp );
      return FALSE;
   }

   while( !done )
   {
      found = FALSE;
      switch( word[1] )
      {
         case 'O':
            if( !strcasecmp( word, "EOF" ) ) { done = TRUE; found = TRUE; break; }
            break;
         case 'A':
            if( !strcmp( word, "#ACCOUNT" ) )
            {
               found = TRUE;
               if( !fread_account( account, fp ) )
                  found = FALSE;
               break;
            }
            break;
         case 'C':
            if( !strcmp( word, "#CHAR_SHEET" ) )
            {
               if( ( cSheet = fread_char_sheet( fp ) ) != NULL )
               {
                  found = TRUE;
                  AttachToList( cSheet, account->characters );
               }
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
         word = ( feof ( fp ) ? FILE_TERMINATOR : fread_word( fp ) );
   }
   fclose( fp );
   return TRUE;
}

void fwrite_account( ACCOUNT *account, FILE *fp )
{
   /* dump the data */
   fprintf( fp, "#ACCOUNT\n" );
   fprintf( fp, "Name              %s~\n", account->name );
   fprintf( fp, "Password          %s~\n", account->password );
   fprintf( fp, "Level             %d\n", account->level );
   fprintf( fp, "PageWidth         %d\n", account->pagewidth );
   fprintf( fp, "#END\n" );
   return;
}

void fwrite_char_sheet( CHAR_SHEET *cSheet, FILE *fp )
{
   fprintf( fp, "#CHAR_SHEET\n" );
   fprintf( fp, "Name            %s~\n", cSheet->name );
   fprintf( fp, "Race            %d\n", cSheet->race );
   fprintf( fp, "Level           %d\n", cSheet->level );
   fprintf( fp, "#END\n" );
   return;
}

bool fread_account( ACCOUNT *account, FILE *fp )
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
            IREAD( "Level", account->level );
            break;
         case 'N':
            SREAD( "Name", account->name );
            break;
         case 'P':
            IREAD( "PageWidth", account->pagewidth );
            SREAD( "Password", account->password );
            break;
      }
      if( !found )
      {
         bug( "%s: bad file format %s", __FUNCTION__, word );
         return FALSE;
      }
      if( !done )
         word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   }
   return TRUE;
}

CHAR_SHEET *fread_char_sheet( FILE *fp )
{
   CHAR_SHEET *cSheet = init_char_sheet();
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
            IREAD( "Level", cSheet->level );
            break;
         case 'N':
            SREAD( "Name", cSheet->name );
            break;
         case 'R':
            IREAD( "Race", cSheet->race );
            break;
      }
      if( !found )
      {
         bug( "%s: bad file format %s", __FUNCTION__, word );
         free_character_sheet( cSheet );
         return NULL;
      }
      if( !done )
         word = ( feof( fp ) ? "#END" : fread_word( fp ) );
   }
   return cSheet;
}

/* Utility */
void account_prompt( D_SOCKET *dsock )
{
   /* should convert to lua later */
   COMMAND *command;
   CHAR_SHEET *character;
   char line[MAX_BUFFER];
   BUFFER *buf = buffer_new(MAX_BUFFER);
   ITERATOR Iter;
   int count = 0;
   int width = dsock->account->pagewidth;

   bprintf( buf, "/%s\\\r\n", print_header( "Account Menu", "-", width - 2 ) );
   AttachIterator(&Iter, dsock->account->commands );
   while( ( command = (COMMAND *)NextInList(&Iter) ) != NULL )
      bprintf( buf, "| %-2d : %-*.*s |\r\n", ++count, width - 9, width - 9, command->cmd_name );
   DetachIterator(&Iter);
   bprintf( buf, "|%s|\r\n", print_header( "Characters", "-", width -2 ) );
   if( SizeOfList( dsock->account->characters ) > 0 )
   {
      AttachIterator( &Iter, dsock->account->characters );
      while( ( character = (CHAR_SHEET *)NextInList( &Iter ) ) != NULL )
      {
         mud_printf( line, "%s the %s", character->name, race_table[character->race] );
         bprintf( buf, "| %-*.*s |\r\n", width - 4, width - 4, character->name, race_table[character->race] );
      }
      DetachIterator( &Iter );
   }
   else
      bprintf( buf, "| %-*.*s |\r\n", width - 4, width - 4, "You have no characters created" );

   bprintf( buf, "\\%s/\r\n", print_header( "End Menu", "-", width - 2 ) );

   bprintf( buf, "\r\nWhat is your choice?: " );

   text_to_buffer( dsock, buf->data );
   buffer_free( buf );
   return;
}

ACCOUNT *check_account_reconnect(const char *act_name)
{
  ACCOUNT *account;
  ITERATOR Iter;

  AttachIterator(&Iter, account_list);
  while ((account = (ACCOUNT *) NextInList(&Iter)) != NULL)
  {
    if (!strcasecmp(account->name, act_name))
    {
      if (account->socket)
        close_socket(account->socket, TRUE);

      break;
    }
  }
  DetachIterator(&Iter);

  return account;
}

void load_account_commands( ACCOUNT *account )
{
   clear_account_command_list( account );
   load_commands( account->commands, tabCmd, STATE_ACCOUNT, account->level );
   return;
}


bool char_list_add( ACCOUNT *account, D_MOBILE *player )
{
   CHAR_SHEET *cSheet;

   if( !player )
   {
      bug( "%s: trying to add a NULL player to %s's account.", __FUNCTION__, account->name );
      return FALSE;
   }
   if( SizeOfList( account->characters ) >= MAX_CHARACTER )
   {
      bug( "%s: trying to add a char to a full char_list on account: %s", __FUNCTION__, account->name );
      return FALSE;
   }
   cSheet = create_char_sheet( player );
   AttachToList( cSheet, account->characters );
   return TRUE;
}

bool char_list_remove( ACCOUNT *account, D_MOBILE *player )
{
   CHAR_SHEET *cSheet;
   ITERATOR Iter;

   if( !player )
   {
      bug( "%s: trying to remove a NULL player from %s's char list.", __FUNCTION__, account->name );
      return FALSE;
   }

   if( SizeOfList( account->characters ) <= 0 )
   {
      bug( "%s: trying to remove a player from a list that is empty on %s's account.", __FUNCTION__, account->name );
      return FALSE;
   }

   AttachIterator( &Iter, account->characters );
   while( ( cSheet = (CHAR_SHEET *)NextInList( &Iter ) ) != NULL )
      if( !strcmp( cSheet->name, player->name ) )
      {
         DetachFromList( cSheet, account->characters );
         DetachIterator( &Iter );
         free_character_sheet( cSheet );
         return TRUE;
      }
   DetachIterator( &Iter );
   return FALSE;
}

ACCOUNT *get_account_from_name( const char *name )
{
   ACCOUNT *account;
   ITERATOR Iter;

   AttachIterator( &Iter, account_list );
   while( ( account = (ACCOUNT *)NextInList( &Iter ) ) != NULL )
      if( !strcmp( account->name, name ) )
         break;
   DetachIterator( &Iter );
   return account;
}

/* retrieval */
const char *get_loc_from_char_sheet( CHAR_SHEET *cSheet )
{
   static char buf[MAX_BUFFER];

   buf[0] = '\0';

   switch( cSheet->level )
   {
      case LEVEL_PLAYER:
         mud_printf( buf, "../instances/mobiles/players/%s.pfile", capitalize( cSheet->name ) );
         return buf;
      case LEVEL_ADMIN:
         mud_printf( buf, "../instances/mobiles/admins/%s.pfile", capitalize( cSheet->name ) );
         return buf;
      case LEVEL_GOD:
         mud_printf( buf, "../instances/mobiles/gods/%s.pfile", capitalize( cSheet->name ) );
         return buf;
   }
   return buf;
}

/* set_account
   Using this command autosaves the account */

void set_account( ACCOUNT *account, unsigned long int value, int type )
{
   switch( type )
   {
      default:
         bug( "%s: invalid type %s passed.", __FUNCTION__, type );
         return;
      case ACT_SOCKET:
         account->socket = (D_SOCKET *)value;
         break;
      case ACT_LEVEL:
         account->level = (sh_int)value;
         break;
      case ACT_CHARACTERS:
         clear_char_sheet_list( account );
         FreeList( account->characters );
         account->characters = (LIST *)value;
         break;
      case ACT_NAME:
         if( account->name )
            free( account->name );
         account->name = strdup( (char *)value );
         break;
      case ACT_PASSWORD:
         if( account->password )
            free( account->password );
         account->password = strdup( (char *)value );
         break;
      case ACT_COMMANDS:
         clear_account_command_list( account );
         FreeList( account->commands );
         account->commands = (LIST *)value;
         break;
      case ACT_PAGEWIDTH:
         account->pagewidth = (sh_int)value;
         break;
      case ACT_SETTINGS:
         account->settings_loaded = (bool)value;
         break;
   }
   save_account( account );
   return;
}


/********************
 * Account Commands *
 ********************/
void act_quit( void *passed, char *arg )
{
   ACCOUNT *account = (ACCOUNT *)passed;
   char buf[MAX_BUFFER];

   /* create a logout string */
   snprintf( buf, MAX_BUFFER, "%s has left the game.", account->name );
   log_string( buf );

   close_socket( account->socket, FALSE );
   return;
}

void act_create_char( void *passed, char *arg )
{
   ACCOUNT *account = (ACCOUNT *)passed;

   if( SizeOfList( account->characters ) >= 3 )
   {
      text_to_buffer( account->socket, "You don't have any free character slots.\r\n" );
      return;
   }
   if( account->socket->nanny )
   {
      bug( "%s: %s attemptings to create a new nanny when one already exists, can't allow this.", __FUNCTION__, account->name );
      text_to_buffer( account->socket, "You cannot do that.\r\n" );
      return;
   }
   change_socket_state( account->socket, STATE_NANNY );
   account->socket->nanny = init_nanny( NANNY_CREATE_CHARACTER );
   account->socket->nanny->socket = account->socket; /* hook the nanny up to our socket for messaging */
   change_nanny_state( account->socket->nanny, NANNY_ASK_CHARACTER_NAME, TRUE );
   return;
}

void act_pagewidth( void *passed, char *arg )
{
   ACCOUNT *account = (ACCOUNT *)passed;
   int width;
   if( !account )
   {
      bug( "%s: passed a NULL account.", __FUNCTION__ );
      return;
   }
   if( !arg || arg[0] == '\0' )
   {
      act_printf( account, "Proper Usage: pagewidth #\r\n" );
      return;
   }
   if( !is_number( arg ) )
   {
      act_printf( account, "You must put in a number.\r\n" );
      return;
   }
   width = atoi( arg );
   set_account( account, width, ACT_PAGEWIDTH );
   act_printf( account, "Width set to %d.\r\n", width );
   return;
}

void act_settings( void *passed, char *arg )
{
   ACCOUNT *account = (ACCOUNT *)passed;
   LIST *flags = AllocList();
   CMD_FLAG *cmdFlag;
   char arg_rem[MAX_BUFFER];

   if( !arg || arg[0] == '\0' )
   {
      act_printf( account, "Proper Usage: settings <flags>\r\n - valid flags: -open -close\r\n" );
      return;
   }

   pull_flags( flags, arg, arg_rem );

   if( ( cmdFlag = get_flag( flags, "-open" ) ) != NULL )
      log_string( "Not NULL." );

   if( SizeOfList( flags ) > 1 )
   {
      act_printf( account, "Settings only takes one flag.\r\n" );
      free_flag_list( flags );
      return;
   }


   if( ( cmdFlag = get_flag( flags, "-open" ) ) != NULL && !account->settings_loaded )
   {
      act_printf( account, "Opening Settings.\r\n" );
      load_commands( account->commands, actSettingCmd, account->socket->state, account->level );
      set_account( account, TRUE, ACT_SETTINGS );
   }
   else if( cmdFlag && account->settings_loaded )
      act_printf( account, "Settings already open.\r\n" );
   else if( ( cmdFlag = get_flag( flags, "-close" ) ) != NULL && account->settings_loaded )
   {
      act_printf( account, "Closing Settings.\r\n" );
      unload_commands( account->commands, actSettingCmd, account->socket->state, account->level );
      set_account( account, FALSE, ACT_SETTINGS );
   }
   else if( cmdFlag && !account->settings_loaded )
      act_printf( account, "Settings already closed.\r\n" );
   else
      act_printf( account, "Invalid Flags Used.\r\n" );

   free_flag_list( flags );
   return;
}

