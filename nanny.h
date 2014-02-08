/* File - nanny.h
   Header file for the nannys that will be created to control things like, character creation, etc
   A nanny is something that takes and guides input as opposed to something that operates from commands
   It's organized, but hardcoded and process potentially raw input.
   Written by Davenge */

struct the_nanny
{
   D_SOCKET *socket;
   sh_int state;
   sh_int type;
   void *creation;
};

/********************
 * Utility  Methods *
 ********************/

/* creation */
NANNY *init_nanny( int type );
void clear_nanny( NANNY *nanny );

/* deletion */
void unload_nanny( NANNY *nanny );
void free_nanny( NANNY *nanny );

/* general utility */
D_MOBILE *nanny_to_player( NANNY *nanny );
void change_nanny_state( NANNY *nanny, int state, bool message );
void nanny_handle_input( D_SOCKET *dsock, char *arg );

/* Character Creation */
void nanny_create_character( D_SOCKET *dsock, char *arg );
void nanny_ask_character_name( D_SOCKET *dsock, char *arg );
void nanny_additional_password( D_SOCKET *dsock, char *arg );
void nanny_confirm_password( D_SOCKET *dsock, char *arg );
void nanny_pick_race( D_SOCKET *dsock, char *arg );
void nanny_complete_character( D_SOCKET *dsock );

/* CHaracter password Checker */
void nanny_character_password_check( D_SOCKET *dsock, char *arg );
