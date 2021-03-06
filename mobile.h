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
   char          * name;
   char          * account; /* name of account this mobile belongs to */
   char          * password;
   sh_int        level;
   sh_int        race;

   /***************
    * New Content *
    ***************/

   /* Utility Variables */
   FRAMEWORK *framework;
   ENTITY *ent_wrapper;
   I_ID *id;
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

/* utility */
void load_mobile_commands args( ( D_MOBILE *dMob ) );
void mobile_prompt( D_SOCKET *dsock );

/* Movement */
bool char_to_game( D_MOBILE *dMob );
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
void cmd_open_workspace( void *passed, char *arg );
void cmd_close_workspace( void *passed, char *arg );
void cmd_pagewidth( void *passsed, char *arg );
/* to code */
void cmd_create_framework( void *passed, char *arg );
