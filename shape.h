/* Shape.h Written by Davenge */

extern LIST *all_shapes;

struct shape
{
   char *name;
   int max_x;
   int max_y;
   int max_z;
   LIST *offsets;
};

/* creation */
SHAPE *init_shape( void );
void clear_shape( SHAPE *shape );

/* deletion */
void free_shape( SHAPE *shape );

/* i/o */
bool load_shapes();
void save_shape( SHAPE *shape );
bool load_shape( const char *location, SHAPE *shape );
void fwrite_shape( SHAPE *shape, FILE *fp );
bool fread_shape( SHAPE *shape, FILE *fp );

