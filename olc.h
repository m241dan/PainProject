/* File olc.h
   This is the header file for olc.c
   Written by Davenge */

/* glboal variables */
extern LIST *workspaces;

struct workspace
{
   char *name;
   char *creator;
   time_t created_on;
   char *last_modifier;
   time_t last_modified;
   int type; /* public, private, etc */

   LIST *contents;

};


/******************
 * Utility Method *
 ******************/

void load_workspaces( void );
WORKSPACE load_workspace( char *location );
void save_workspaces( void );
void save_workspace( D_MOBILE *dMob, WORKSPACE *wspace );
bool check_work( D_MOBILE *dMob );
void add_frame_to_workspace( FRAMEWORK *frame, D_MOBILE *dMob );

