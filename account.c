/* File account.c
   All functions/methods corresponding with accounts
   Written by Davenge */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "mud.h"

ACCOUNT *load_account( const char *act_name, bool partial )
{
   FILE *fp;
   ACCOUNT *account = NULL;
   char *word;
   char aFile[MAX_BUFFER];
   bool done = FALSE, found;

   snprintf( aFile, MAX_BUFFER, "../accounts/%s/account.afile", capitalize( act_name ) );
   if( ( fp = fopen( aFile, "r" ) ) == NULL )
      return NULL;

   /* grab an account from the stack or create a new one */
   if( StackSize( account_free ) <= 0 )
      CREATE( account, ACCOUNT, 1 );
   else
      account = (ACCOUNT *)PopStack(account_free);

   clear_account(account);
   account->loaded = TRUE;

   word = fread_word( fp );
   while( !done )
   {
      found = FALSE;
      switch( word[0] )
      {
         case 'E':
            if( !strcasecmp( word, "EOF" ) )
            {
               done = TRUE;
               found = TRUE;
               break;
            }
            break;
         case 'L':
            IREAD( "Level", account->level );
            break;
         case 'N':
            SREAD( "Name", account->name );
            break;
         case 'P':
            if( !strcmp( word, "Password" ) )
            {
               account->password = fread_string( fp );
               found = TRUE;
               if( partial )
               {
                  done = TRUE;
                  account->loaded = FALSE;
               }
               break;
            }
            break;
      }
      if( !found )
      {
         bug( "Load_account: unexpected '%s' in %s's aFile.", word, act_name );
         free_account(account);
         return NULL;
      }

      if( !done )
         word = fread_word( fp );
   }
   fclose( fp );
   return account;
}

void fwrite_account( ACCOUNT *account )
{
   FILE *fp;
   char aDir[MAX_BUFFER], aFile[MAX_BUFFER];

   /* create the "file directory" for the account, so we can check if it exists */
   snprintf( aDir, MAX_BUFFER, "../accounts/%s", capitalize( account->name ) );

   if( opendir( aDir ) == NULL )
   {
      if( ( mkdir( aDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ) ) != 0 ) /* was unsuccessful */
      {
         bug( "Unable to create folder for the new account: %s", account->name );
         return;
      }
   }

   /* create the "file name" for the account */
   snprintf( aFile, MAX_BUFFER, "%s/account.afile", aDir );

   /* open the pointer to the file, if we can't, spit out a bug msg and return */
   if( ( fp = fopen( aFile, "w" ) ) == NULL )
   {
      bug( "Unable to write to %s's account file", account->name );
      return;
   }

   /* dump the data */
   fprintf( fp, "Name              %s~\n", account->name );
   fprintf( fp, "Password          %s~\n", account->password );
   /* don't put anything above this point unless you want it included in the partial load -Davenge */

   fprintf( fp, "Level             %d\n", account->level );

   /* make sure file has a terminator on it */
   fprintf( fp, "%s\n", FILE_TERMINATOR );
   fclose( fp );

}

void clear_account( ACCOUNT *account )
{
   account->name = NULL;
   account->password = NULL;
   account->level = LEVEL_BASIC;
   account->commands = AllocList();
   return;
}

void free_account( ACCOUNT *account )
{
   DetachFromList( account, account_list );

   if( account->socket )
      account->socket->player = NULL;

   clear_command_list( account );
   free( account->name );
   free( account->password );

   PushStack( account, account_free );
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

void account_prompt( D_SOCKET *dsock )
{
   /* should convert to lua later */
   COMMAND *command;
   BUFFER *buf = buffer_new(MAX_BUFFER);
   ITERATOR Iter;
   int count = 0;

   bprintf( buf, "%s Account Menu %s\r\n", produce_equals(30), produce_equals(30) );
   AttachIterator(&Iter, dsock->account->commands );
   while( ( command = (COMMAND *)NextInList(&Iter) ) != NULL )
      bprintf( buf, "%d : %s\r\n", ++count, command->cmd_name );
   DetachIterator(&Iter);
   text_to_buffer( dsock, buf->data );
   buffer_free( buf );
   return;
}

void act_quit( void *passed, char *argument )
{
   ACCOUNT *account = (ACCOUNT *)passed;
   char buf[MAX_BUFFER];

   /* create a logout string */
   snprintf( buf, MAX_BUFFER, "%s has left the game.", account->name );
   log_string( buf );

   account->socket->account = NULL;
   free_account( account );
   close_socket( account->socket, FALSE );
   return;
}

void load_commands( ACCOUNT *account )
{
   COMMAND *com;
   int x;

   if( account->commands )
   {
      clear_command_list( account ); /* clear it out before we lost new commands */
      account->commands = AllocList();
   }

   for( x = 0; tabCmd[x].cmd_name[0] != '\0'; x++ ) /* load the new commands that fit our criteria */
      if( tabCmd[x].state == STATE_ACCOUNT && tabCmd[x].level <= account->level )
      {
         com = copy_command( tabCmd[x] );
         AttachToList( com, account->commands );
      }
   return;
}

void clear_command_list( ACCOUNT *account )
{
   COMMAND *com;
   ITERATOR Iter;

   AttachIterator(&Iter, account->commands );
   while( ( com = (COMMAND *)NextInList(&Iter) ) != NULL )
      free_command( com );
   DetachIterator(&Iter);
   FreeList( account->commands );
   return;
}
