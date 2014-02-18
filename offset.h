/* Offset.h Written by Davenge */

struct offset
{
   int x;
   int y;
   int z;
};

/* creation */
OFFSET *init_offset();
void clear_offset( OFFSET *off );
/* deletion */
void free_offset( OFFSET *off );
void free_offset_list( LIST *offsets );

/* i/o */
void fwrite_offset( OFFSET *off, FILE *fp );
OFFSET *fread_offset( FILE *fp );
