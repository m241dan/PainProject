/* File room.h
   This is the header file for room.c
   Written by Davenge */


struct room
{
   FRAMEWORK *framework;
   ENTITY *ent_wrapper;
   I_ID *id;

   char *name;
   char *short_descr;
   char *long_descr;
   char *title;
   char *description;
   bool inside;
};

/* creation */
ROOM *init_room( void );
void clear_room( ROOM *room );
ROOM *create_room( D_MOBILE *dMob, FRAMEWORK *fWork);

/* delete */
void free_room( ROOM *room );
void delete_room( ROOM *room );

/* i/o */
bool load_rooms( void );
void save_room( ROOM *room );
bool load_room( const char *location, ROOM *room );
void fwrite_room( ROOM *room, FILE * fp );
void fread_room( ROOM *room, FILE *fp );

/* retrieval */
ROOM *get_room( const char *name );
