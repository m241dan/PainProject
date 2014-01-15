/* File mobile.h
   This is the header file for mobile.c
   Written by Davenge */

/**********
 * Macros *
 **********/
#define IS_NPC(dMob)          ((dMob->level) == LEVEL_NPC ? TRUE : FALSE)

/***************
 * Definitions *
 ***************/

struct dMobile
{
   D_SOCKET      * socket;
   LIST          * events; /*game data */
   char          * name; /* account data */
   char          * password; /* account data */
   sh_int        level; /* account data */

   /***************
    * New Content *
    ***************/

   /* Utility Variables */
   bool loaded;
   ACCOUNT *account;

   /* account data */
   sh_int  race;

   /* game data */
   ENTITY *ent_wrapper;
   LIST *commands;
   COORD *at_coord;
};


/*******************
 * Utility Methods *
 *******************/
void save_mobile( D_MOBILE *mobile );
void fwrite_account_data( D_MOBILE *dMob);
void fwrite_game_data( D_MOBILE *dMob );
void fread_mobile_account_data( const char *pFile, D_MOBILE *dMob );
void fread_mobile_game_data( const char *gFile, D_MOBILE *dMob );
void save_player( D_MOBILE *mobile );
void load_mobile( ACCOUNT *account, char *player, bool partial, D_MOBILE *dMob );
void unload_mobile( D_MOBILE *dMob, bool partial );
void alloc_mobile_lists( D_MOBILE *dMob );
void free_mobile_lists( D_MOBILE *dMob );
void free_mobile_game_data(D_MOBILE *dMob );
void free_mobile_account_data( D_MOBILE *dMob );
void clear_mobile( D_MOBILE *dMob );
void load_player( ACCOUNT *account, char *player, bool partial, D_MOBILE *dMob );
void load_mobile_commands args( ( D_MOBILE *dMob ) );
void clear_mobile_command_list args( ( D_MOBILE *dMob ) );
void clear_mobile_event_list( D_MOBILE *dMob );
void char_to_game( D_SOCKET *dsock, D_MOBILE *dMob );
void mob_from_coord( D_MOBILE *dMob );
void mob_to_coord( D_MOBILE *dMob, COORD *coordinate );
void move_char( D_MOBILE *dMob, int dir );

/**************************
 * Mobile Command Methods *
 **************************/

void cmd_look( void *passed, char *arg );
void cmd_north( void *passed, char *arg );
void cmd_east( void *passed, char *arg );
void cmd_south( void *passed, char *arg );
void cmd_west( void *passed, char *arg );
void cmd_up( void *passed, char *arg );
void cmd_down( void *passed, char *arg );
