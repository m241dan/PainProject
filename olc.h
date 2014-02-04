/* File olc.h
   This is the header file for olc.c
   Written by Davenge */

/* glboal variables */
extern LIST *workspaces;

struct workspace
{
   char *name;
   int type; /* public, private, etc */
   I_ID *id;

   LIST *contents; /* frameworks */

};


/******************
 * Utility Method *
 ******************/

/* creation */
WORKSPACE *init_workspace( void );
/* deletion */
void free_workspace( WORKSPACE *wSpace );
/* i/o */
bool load_workspaces( void );
void save_workspace( WORKSPACE *wSpace );
bool load_workspace( const char *location, WORKSPACE *wSpace );
void fwrite_workspace( WORKSPACE *wSpace, FILE *fp );
void fread_workspace( WORKSPACE *wSpace, FILE *fp );

/* utility */
bool check_work( D_MOBILE *dMob );
void add_frame_to_workspace( FRAMEWORK *frame, D_MOBILE *dMob );
