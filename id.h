/* File id.h
   This is the header file for id.c
   Written by Davenge */

extern LIST *id_handlers;

typedef enum
{
   RFRAME_HANDLER, MAX_ID_HANDLER
} id_handler_types;

struct id_handler
{
   LIST *free_ids;
   int type;
   int top_id;
};

struct instance_id
{
   int id;
   char *created_by;
   char *created_on;
   char *modified_by;
   char *last_modified;
};


/*******************
 * Utility Methods *
 *******************/
/* onboot */
bool load_id_handlers();
/* creation */
ID_HANDLER *init_id_handler( int type );
I_ID *create_raw_id( int id );
I_ID *create_new_id( D_MOBILE *dMob, int type );
/* deletion */
void free_id_handler( ID_HANDLER *handler );
void free_i_id( I_ID *id );
/* i/o */
void save_id_handler( ID_HANDLER *handler );
bool load_id_handler( ID_HANDLER *handler );
void fwrite_id_handler( ID_HANDLER *handler, FILE *fp );
void fread_id_handler( ID_HANDLER *handler, FILE *fp );
void fwrite_i_id( I_ID *id, FILE *fp );
I_ID *fread_i_id( FILE *fp );
/* utility */
ID_HANDLER *get_id_handler( int type );
I_ID *check_free( ID_HANDLER *handler );
int get_top_id( ID_HANDLER *handler );
void update_id( D_MOBILE *dMob, I_ID *id );

