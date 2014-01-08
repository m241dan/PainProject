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
  LIST          * events;
  char          * name;
  char          * password;
  sh_int        level;
   /* New Content */
   bool loaded;
   ACCOUNT *account;
   sh_int  race;
};


/*******************
 * Utility Methods *
 *******************/
void save_mobile( D_MOBILE *mobile );
void save_player( D_MOBILE *mobile );
void free_mobile(D_MOBILE *dMob);
void clear_mobile(D_MOBILE *dMob);
D_MOBILE *load_player( ACCOUNT *account, char *player, bool partial );
