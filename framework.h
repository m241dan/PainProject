/* File framework.h
   This is the header file for framework.c
   Written by Davenge */


typedef enum {
   FRAME_ROOM, MAX_FRAME
} framework_type_enum;


struct framework
{
   void *content;
   int type;
   time_t created_on;
   time_t last_modified;
   char *created_by;
   char *modified_by;
};

struct room_framework
{
   I_ID *id;
   char *title;
   char *description;
};

/*******************
 * Utility Methods *
 *******************/

/* creation */
FRAME_WORK *create_framework( D_MOBILE *dMob, int type );
