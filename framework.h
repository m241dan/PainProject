/* File framework.h
   This is the header file for framework.c
   Written by Davenge */

extern LIST *all_frameworks;

typedef enum
{
   ROOM_FRAME, MAX_FRAMEWORK
} framework_name_ids;

struct framework
{
   void *content;
   int type;
   I_ID *id;
};

struct room_framework
{
   char *title;
   char *description;
};

/*******************
 * Utility Methods *
 *******************/

/* general */
/* creation */
FRAMEWORK *create_framework( D_MOBILE *dMob, int type );
/* i/o */
void load_frameworks( void );
void save_frameworks( void );
void save_framework( D_MOBILE *dMob, FRAMEWORK *fwork );
void load_framework( char *location );
/* room specific */
/* creation */
R_FRAMEWORK *create_rFramework( void );
/* i/o */
R_FRAMEWORK *load_rFramework( char *location );
void save_rFramework( R_FRAMEWORK *rFrame );

/* checking */
bool valid_ftype( int type );
