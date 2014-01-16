/* File id.h
   This is the header file for id.c
   Written by Davenge */

struct id_handler
{
   LIST *free_ids;
   int top_id;
};

struct instance_id
{
   int id;
   time_t created_on;
   time_t last_modified;
};


/*******************
 * Utility Methods *
 *******************/

ID_HANDLER *init_id_handler( void );
void load_rid_handler( void );
void save_rid_handler( void );
void fwrite_id_handler( ID_HANDLER *handler, char *location );
void fread_id_handler( ID_HANDLER *handler, char *location );
I_ID *create_raw_id( int id, int create, int modify );
I_ID *create_new_id( int type );
I_ID *check_free( ID_HANDLER *handler );
int get_top_id( ID_HANDLER *handler );
