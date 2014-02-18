/* creator.c Written by Davenge */

#include "mud.h"

/* creation */
CREATOR *init_creator( void )
{
   CREATOR *creator;

   CREATE( creator, CREATOR, 1 );
   clear_creator( creator );
   return creator;
}

void clear_creator( CREATOR *creator )
{
   creator->cShape = NULL;
   creator->to_create = NULL;
   return;
}

/* deletion */

void ree_creator( CREATOR *creator )
{
   creator->cShape = NULL;
   creator->to_create = NULL;
   free( creator );
}
