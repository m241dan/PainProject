/* File framework.h
   This is the header file for framework.c
   Written by Davenge */

extern ID_HANDLER global_id;


typedef enum {
   FRAME_ROOM, MAX_FRAME
} framework_type_enum;


struct framework_id_handler
{
   LIST *free_ids;
   int top_id;
}

struct frame_id
{
   int id;
   time_t created_on;
   time_t last_modified;
}

struct framework
{
   void *content;
   int type;
};

struct room_framework
{
   FRAME_ID fid;
   char *title;
   char *description;
};

/*******************
 * Utility Methods *
 *******************/
void init_id_handler( void );
void fwrite_id_handler( void );
void fread_id_handler( void );
