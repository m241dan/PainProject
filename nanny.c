/* File nanny.c
   All functions/methods corresponding with nannys
   Written by Davenge */

#include <stdio.h>
#include "mud.h"

/******************
 * UILITY METHODS *
 ******************/

/* This method is for freeing up a nanny once its done its job -Davenge */
void free_nanny( NANNY *nanny )
{
   nanny->socket->nanny = NULL;
   nanny->socket = NULL;
   nanny->creation = NULL;
   free(nanny);
   return;
}

/* This method changes the state of the nanny and spits out the corresponding message -Davenge */
void change_nanny_state( NANNY *nanny, int state, bool message )
{
   char output[MAX_BUFFER];

   nanny->state = state;

   if( !message )
      return;

   snprintf( output, MAX_BUFFER, "%s\r\n", nanny_strings[nanny->type][nanny->state] );
   text_to_buffer( nanny->socket, output );
   return;
}

/* This method universally creates nannys of the given types and places the correct structures in the creation pointer -Davenge */
NANNY *create_nanny( D_SOCKET *dsock, int type )
{
   NANNY *nanny;

   CREATE( nanny, NANNY, 1 );
   nanny->state = 0;
   nanny->type = type;
   nanny->socket = dsock;
   switch( type )
   {
      case NANNY_CREATE_CHARACTER:
         CREATE( nanny->creation, D_MOBILE, 1 );
         ((D_MOBILE *)nanny->creation)->level = LEVEL_PLAYER;
         break;
   }
   return nanny;
}

/***********************
 * END UTILITY METHODS *
 ***********************/

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
   }
}


/* These are the specific data handling methods called by the control method */

/* This method processes the characters name -Davenge */
void nanny_ask_character_name( D_SOCKET *dsock, char *arg )
{
   D_MOBILE *player = (D_MOBILE *)dsock->nanny->creation;
   char name[MAX_BUFFER];

   arg = one_arg( arg, name );

   player->name = strdup( arg );
   change_nanny_state( dsock->nanny, NANNY_ADDITIONAL_PASSWORD, TRUE );
   text_to_buffer(dsock, (char *) dont_echo); /* hardcoded laziness, last time, I swear (will figure out and fix later, promise) -Davenge */
   return;
}

void nanny_additional_password( D_SOCKET *dsock, char *arg )
{
   D_MOBILE *player = (D_MOBILE *)dsock->nanny->creation;
   char passwd[MAX_BUFFER];

   arg = one_arg( arg, passwd );

   if( passwd[0] == '\0' ) /* no password? no problem, create player */
   {
      text_to_buffer(dsock, (char *) do_echo);
      nanny_complete_character( dsock );
      return;
   }
   player->password = strdup(crypt(passwd, dsock->account->name));
   change_nanny_state( dsock->nanny, NANNY_CONFIRM_ADDITIONAL_PASSWORD, TRUE );
   text_to_buffer(dsock, (char *) dont_echo);
   return;
}

void nanny_confirm_password( D_SOCKET *dsock, char *arg )
{
   D_MOBILE *player = (D_MOBILE *)dsock->nanny->creation;
   char passwd[MAX_BUFFER];

   arg = one_arg( arg, passwd );
   text_to_buffer(dsock, (char *) do_echo);
   if( !strcmp(crypt(passwd, dsock->account->name), player->password) )
   {
      nanny_complete_character( dsock );
      return;
   }
   text_to_buffer( dsock, "Passwords don't match.\r\n" );
   free(player->password);
   change_nanny_state( dsock->nanny, NANNY_ADDITIONAL_PASSWORD, TRUE );
   return;
}

/* The finalize method for character creation, all good nannys should have a finalize method
 * You know, where the do what they are supposed to do with the thing that they have created
 * and send the proper message to the player and change their socket state back to normal
   -Davenge */
void nanny_complete_character( D_SOCKET *dsock )
{
   D_MOBILE *new_char = (D_MOBILE *)dsock->nanny->creation;
   text_to_buffer( dsock, "New Character Successfully Created\r\n\r\n" );
   free(new_char->name);
   free(new_char->password);
   free(new_char);
   free_nanny( dsock->nanny );
   dsock->state = STATE_ACCOUNT;
   return;
}

/********************************
 * END CHARACTER CREATION NANNY *
 ********************************/
