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
   /* New Content */
   bool loaded; /* utility variable */
   ACCOUNT *account;

   /* account data */
   sh_int  race;

   /* game data */
   LIST *commands;

};


/*******************
 * Utility Methods *
 *******************/
void save_mobile( D_MOBILE *mobile );
void save_player( D_MOBILE *mobile );
void unload_mobile( D_MOBILE *dMob, bool partial );
void free_mobile_game_data(D_MOBILE *dMob );
void free_mobile_account_data( D_MOBILE *dMob );
void clear_mobile(D_MOBILE *dMob);
D_MOBILE *load_player( ACCOUNT *account, char *player, bool partial );
void load_mobile_commands args( ( D_MOBILE *dMob ) );
void clear_mobile_command_list args( ( D_MOBILE *dMob ) );
void clear_mobile_event_list( D_MOBILE *dMob );
