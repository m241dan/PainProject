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
   sh_int        race; 

   /***************
    * New Content *
    ***************/

   /* Utility Variables */
   ACCOUNT *account;
   ENTITY *ent_wrapper;
   LIST *commands;
   COORD *at_coord;

   /* olc stuff */
   WORKSPACE *workspace;

};


/*******************
 * Utility Methods *
 *******************/

/* creation */
D_MOBILE *init_mobile( void );
void clear_mobile( D_MOBILE *dMob );

/* deletion */
void unload_mobile( D_MOBILE *dMob );
void free_mobile( D_MOBILE *dMob );
void clear_mobile_event_list( D_MOBILE *dMob );
void clear_mobile_command_list( D_MOBILE *dMob );

/* i/o */
void save_mobile( D_MOBILE *dMob );
bool load_mobile( const char *location, D_MOBILE *dMob );
void fwrite_mobile( D_MOBILE *dMob, FILE *fp );
bool fread_mobile( D_MOBILE *dMob, FILE *fp );

/* I/O */
void save_mobile( D_MOBILE *mobile );
void fwrite_account_data( FILE *fp, D_MOBILE *dMob);
void fwrite_game_data( FILE *fp, D_MOBILE *dMob );
void fread_mobile_account_data( FILE *fp, D_MOBILE *dMob );
void fread_mobile_game_data( FILE *fp, D_MOBILE *dMob );
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

/* Movement */
void char_to_game( D_SOCKET *dsock, D_MOBILE *dMob );
void mob_from_coord( D_MOBILE *dMob );
void mob_to_coord( D_MOBILE *dMob, COORD *coordinate );
void move_char( D_MOBILE *dMob, int dir );

/**************************
 * Mobile Command Methods *
 **************************/

/* Utility */
void cmd_look( void *passed, char *arg );

/* Movement */
void cmd_north( void *passed, char *arg );
void cmd_east( void *passed, char *arg );
void cmd_south( void *passed, char *arg );
void cmd_west( void *passed, char *arg );
void cmd_up( void *passed, char *arg );
void cmd_down( void *passed, char *arg );

/* Building */
void cmd_load( void *passed, char *arg );
