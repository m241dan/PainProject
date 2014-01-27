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
FRAMEWORK *create_framework( D_MOBILE *dMob, int type );\
/* deletion */
void free_framework( FRAMEWORK *frame );
/* i/o */
void save_framework( FRAMEWORK *frame );
bool load_framework( const char *location, FRAMEWORK *frame );
void fwrite_framework( FRAMEWORK *frame, FILE *fp );
void fread_framework( FRAMEWORK *frame, FILE *fp );
/* room specific */
/* creation */
R_FRAMEWORK *create_rFramework( void );
/* deletion */
void free_rFramework( R_FRAMEWORK *rFrame );
/* i/o */
void fwrite_rFramework( R_FRAMEWORK *rFrame, FILE *fp );
R_FRAMEWORK *fread_rFramework( FILE *fp );

/* checking */
bool valid_ftype( int type );
