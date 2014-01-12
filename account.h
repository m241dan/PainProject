/* File - account.h
   Header file for Account system
   Written by Davenge */

#define MAX_CHARACTER 3

#define save_account(account) fwrite_account( account )

/* The levels an account can be */
typedef enum {
   LEVEL_BASIC, LEVEL_ENFORCER, LEVEL_DEVELOPER, LEVEL_OWNER, MAX_ACCOUNT_LEVEL
} account_levels;

struct game_account
{
   /* is the structure fully loaded? */
   bool loaded;

   /* socket controlling this account */
   D_SOCKET *socket;

   /* what level is the account? Admin? Player? Developer? Owner? */
   int level;

   /* the list of characters in an array, max size of MAX_CHARACTER */
   LIST *characters;

   char *name;
   char *password;

   /* account commands container */
   LIST *commands;
};

/* Account Utilities */
void fwrite_account( ACCOUNT *account );
ACCOUNT *load_account( const char *act_name, bool partial );
void clear_account( ACCOUNT *account );
void free_account( ACCOUNT *account );
ACCOUNT *check_account_reconnect( const char *act_name );
void account_prompt( D_SOCKET *dsock );
void load_account_commands args( ( ACCOUNT *account ) );
void clear_account_command_list args( ( ACCOUNT *account ) );
void clear_character_list( ACCOUNT *account );
void char_list_add( ACCOUNT *account, D_MOBILE *player );
void char_list_remove( ACCOUNT *account, D_MOBILE *player );
/* Account Commands */
void act_quit( void *passed, char *argument );
void act_create_char( void *passed, char *argument );

