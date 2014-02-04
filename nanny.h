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

void free_nanny( NANNY *nanny );
void change_nanny_state( NANNY *nanny, int state, bool message );
NANNY *create_nanny( D_SOCKET *dsock, int type );
void nanny_handle_input( D_SOCKET *dsock, char *arg );

/* Character Creation */
void nanny_create_character( D_SOCKET *dsock, char *arg );
void nanny_ask_character_name( D_SOCKET *dsock, char *arg );
void nanny_additional_password( D_SOCKET *dsock, char *arg );
void nanny_confirm_password( D_SOCKET *dsock, char *arg );
void nanny_pick_race( D_SOCKET *dsock, char *arg );
void nanny_complete_character( D_SOCKET *dsock );
