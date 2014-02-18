/* File - account.h
   Header file for Account system
   Written by Davenge */

#define MAX_CHARACTER 3

struct game_account
{
   /* socket controlling this account */
   D_SOCKET *socket;

   /* what level is the account? Admin? Player? Developer? Owner? */
   int level;

   /* the list of characters max size of MAX_CHARACTER */
   LIST *characters;

   char *name;
   char *password;

   /* account commands container */
   LIST *commands;

   /* utility stuff */
   sh_int pagewidth;
   bool settings_loaded;
};

struct character_sheet
{
   char *name;
   int race;
   int level;
};

/*******************
 * Utility Methods *
 *******************/
/* creation */
ACCOUNT *init_account( void );
void clear_account( ACCOUNT *account );
CHAR_SHEET *init_char_sheet( void );
void clear_char_sheet( CHAR_SHEET *cSheet );
CHAR_SHEET *create_char_sheet( D_MOBILE *dMob );

/* deletion */
void unload_account( ACCOUNT *account );
void free_account( ACCOUNT *account );
void free_character_sheet( CHAR_SHEET *cSheet );
void clear_char_sheet_list( ACCOUNT *account );
void clear_account_command_list( ACCOUNT *account );


/* i/o */
bool load_accounts( void );
void save_account( ACCOUNT *account );
bool load_account( const char *location, ACCOUNT *account );
void fwrite_account( ACCOUNT *account, FILE *fp );
void fwrite_char_sheet( CHAR_SHEET *cSheet, FILE *fp );
bool fread_account( ACCOUNT *account, FILE *fp );
CHAR_SHEET *fread_char_sheet( FILE *fp );

/* utility */
void account_prompt( D_SOCKET *dsock );
ACCOUNT *check_account_reconnect( const char *act_name );
void load_account_commands args( ( ACCOUNT *account ) );
bool char_list_add( ACCOUNT *account, D_MOBILE *player );
bool char_list_remove( ACCOUNT *account, D_MOBILE *player );
ACCOUNT *get_account_from_name( const char *name );

/* retrieval */
const char *get_loc_from_char_sheet( CHAR_SHEET *cSheet );
ACCOUNT *get_account( const char *name );

/* setting */
void set_account( ACCOUNT *account, VALUE value, int type );

/********************
 * Account Commands *
 ********************/
void act_quit( void *passed, char *arg );
void act_create_char( void *passed, char *arg );
void act_pagewidth( void *passed, char *arg );
void act_settings( void *passed, char *arg );
