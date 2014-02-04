/* File nanny.c
   All functions/methods corresponding with nannys
   Written by Davenge */

#include <stdio.h>
#include "mud.h"

/******************
 * UILITY METHODS *
 ******************/

/* creation */
NANNY *init_nanny( int type )
{
   NANNY *nanny;

   CREATE( nanny, NANNY, 1 );
   clear_nanny( nanny );
   nanny->type = type;
   switch( type )
   {
      case NANNY_CREATE_CHARACTER:
         nanny->creation = init_mobile();
         ((D_MOBILE *)nanny)->level = LEVEL_PLAYER;
         break;
   }

   return nanny;
}

void clear_nanny( NANNY *nanny )
{
   nanny->socket = NULL;
   nanny->state = -1;
   nanny->type = -1;
   nanny->creation = NULL;
   return;
}


/* deletion */
void unload_nanny( NANNY *nanny )
{
   free_nanny( nanny );
   return;
}

void free_nanny( NANNY *nanny )
{
   if( nanny->socket )
      nanny->socket->nanny = NULL; /* unhook from socket */
   if( nanny->creation )
      nanny->creation = NULL;
   return;
}

/* general utiltiy */

/* This method changes the state of the nanny and spits out the corresponding message -Davenge */
void change_nanny_state( NANNY *nanny, int state, bool message )
{
   char output[MAX_BUFFER];

   nanny->state = state;
   if( ( nanny->state == NANNY_ADDITIONAL_PASSWORD || nanny->state == NANNY_CONFIRM_ADDITIONAL_PASSWORD ) && nanny->type == NANNY_CREATE_CHARACTER )
      text_to_buffer( nanny->socket, (char *) dont_echo );

   if( !message )
      return;

   snprintf( output, MAX_BUFFER, "%s\r\n", nanny_strings[nanny->type][nanny->state] );
   text_to_buffer( nanny->socket, output );
   return;
}


/*************************
 * General Nanny Handler *
 * Written by Davenge    *
 *************************/
void nanny_handle_input( D_SOCKET *dsock, char *arg )
{
   if( !dsock->nanny )
   {
      bug( "%s: bad socket state, in state nanny but no nanny created.", __FUNCTION__ );
      text_to_socket( dsock, "Sorry, something is really messed up.\r\n" );
      dsock->state = STATE_ACCOUNT;
      return;
   }

   if( !strcmp( arg, "/back" ) )
   {
      text_to_buffer(dsock, (char *) do_echo);
      if( ( dsock->nanny->state - 1 ) == NANNY_CONFIRM_ADDITIONAL_PASSWORD )
         change_nanny_state( dsock->nanny, ( dsock->nanny->state - 1), FALSE );
      change_nanny_state( dsock->nanny, ( dsock->nanny->state - 1 ), TRUE );
      return;
   }

   switch( dsock->nanny->type )
   {
      default:
         bug( "%s: Bad Nanny Type: %d", __FUNCTION__, dsock->nanny->type );
         return;
      case NANNY_CREATE_CHARACTER:
         nanny_create_character( dsock, arg );
         return;
   }
}
/***********************/



/****************************
 * CHARACTER CREATION NANNY *
 ****************************/

/* These are methods for the Nanny Create Character Nanny
 * This is the control method for the Nanny Create character Nanny.
 * It is called by the General Nanny Handler
 */
void nanny_create_character( D_SOCKET *dsock, char *arg )
{
   switch( dsock->nanny->state )
   {
      default:
         bug( "%s: Bad Nanny State: %d", __FUNCTION__, dsock->nanny->state );
         return;
      case NANNY_ASK_CHARACTER_NAME:
         nanny_ask_character_name( dsock, arg );
         return;
      case NANNY_ADDITIONAL_PASSWORD:
         nanny_additional_password( dsock, arg );
         return;
      case NANNY_CONFIRM_ADDITIONAL_PASSWORD:
         nanny_confirm_password( dsock, arg );
         return;
      case NANNY_PICK_RACE:
	 nanny_pick_race( dsock, arg );
	 return;
   }
}


/* These are the specific data handling methods called by the control method */

/* This method processes the characters name -Davenge */
void nanny_ask_character_name( D_SOCKET *dsock, char *arg )
{
   D_MOBILE *player = (D_MOBILE *)dsock->nanny->creation;
   char name[MAX_BUFFER];

   arg = one_arg( arg, name );
   capitalize_orig( name );
   player->name = strdup( name );
   change_nanny_state( dsock->nanny, NANNY_ADDITIONAL_PASSWORD, TRUE );
   return;
}

void nanny_additional_password( D_SOCKET *dsock, char *arg )
{
   D_MOBILE *player = (D_MOBILE *)dsock->nanny->creation;
   char passwd[MAX_BUFFER];

   text_to_buffer(dsock, (char *) do_echo);
   arg = one_arg( arg, passwd );
   if( passwd[0] == '\0' ) /* no password? no problem, create player */
   {
      show_race_table( dsock );
      change_nanny_state( dsock->nanny, NANNY_PICK_RACE, TRUE );
      return;
   }
   player->password = strdup(crypt(passwd, dsock->account->name));
   change_nanny_state( dsock->nanny, NANNY_CONFIRM_ADDITIONAL_PASSWORD, TRUE );
   return;
}

void nanny_confirm_password( D_SOCKET *dsock, char *arg )
{
   D_MOBILE *player = (D_MOBILE *)dsock->nanny->creation;
   char passwd[MAX_BUFFER];

   text_to_buffer(dsock, (char *) do_echo);
   arg = one_arg( arg, passwd );
   if( !strcmp(crypt(passwd, dsock->account->name), player->password) )
   {
      show_race_table( dsock );
      change_nanny_state( dsock->nanny, NANNY_PICK_RACE, TRUE );
      return;
   }
   text_to_buffer( dsock, "Passwords don't match.\r\n" );
   free(player->password);
   change_nanny_state( dsock->nanny, NANNY_ADDITIONAL_PASSWORD, TRUE );
   return;
}

/* Wasnt sure on how to make a new list, I forgotteded, so I just did it like names for practice, until I get back and can do it right. -Irish */
void nanny_pick_race( D_SOCKET *dsock, char *arg )
{
   D_MOBILE *player = (D_MOBILE *)dsock->nanny->creation;
   char race[MAX_BUFFER];
   int x;

   arg = one_arg( arg, race );
   downcase_orig( race ); /* we've got to make sure the input matches what we will compare it to */
   for( x = 0; x < MAX_RACE; x++ )
   {
      if( !strcmp( race, downcase( smash_color( race_table[x] ) ) ) ) /* make sure the entry from the table is lowercased to match our string and color codes are removed */
      {
         player->race = x;
         break;
      }
   }

   if( x >= MAX_RACE ) /* X will be Max_race if it is not found */
   {
      text_to_buffer( dsock, "No such race.\r\n" );
      change_nanny_state( dsock->nanny, NANNY_PICK_RACE, TRUE ); /* basically just spit out the pick race mesage again */
      return;
   }

   nanny_complete_character( dsock );
   return;
}

/* The finalize method for character creation, all good nannys should have a finalize method
 * You know, where the do what they are supposed to do with the thing that they have created
 * and send the proper message to the player and change their socket state back to normal
   -Davenge */
void nanny_complete_character( D_SOCKET *dsock )
{
   D_MOBILE *new_char = (D_MOBILE *)dsock->nanny->creation;

   if( !new_char )
   {
      bug( "%s: being called but the creation is NULL... wtf: account %s", __FUNCTION__, dsock->account->name );
      return;
   }

   text_to_buffer( dsock, "New Character Successfully Created\r\n\r\n" );
   char_list_add( dsock->account, new_char );
   save_mobile( new_char );
   unload_mobile( new_char );

   unload_nanny( dsock->nanny );
   change_socket_state( dsock,  STATE_ACCOUNT );
   return;
}

/********************************
 * END CHARACTER CREATION NANNY *
 ********************************/

