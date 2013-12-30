/* File - nanny.h
   Header file for the nannys that will be created to control things like, character creation, etc
   A nanny is something that takes and guides input as opposed to something that operates from commands
   It's organized, but hardcoded and process potentially raw input.
   Written by Davenge */

#define MAX_NANNY_STATES 10

typedef enum {
   NANNY_CREATE_CHARACTER, MAX_NANNY_TYPE
} nanny_types;

typedef enum {
   NANNY_ASK_CHARACTER_NAME, NANNY_ADDITIONAL_PASSWORD, MAX_CHARACTER_NANNY_STATE
} character_nanny_states;

struct the_nanny
{
   sh_int state;
   sh_int type;
   void *creation;
};
void nanny_handle_input( D_SOCKET *dsock, char *arg );
void nanny_create_character( D_SOCKET *dsock, char *arg );
void nanny_ask_character_name( D_SOCKET *dsock, char *arg );
