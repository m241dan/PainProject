/* File olc.c
   All methods pertaining to the olc go here
   Written by Davenge */

#include "mud.h"




/*******************
 * Utility Methods *
 *******************/

bool check_work( D_MOBILE *dMob )
{
   if( !dMob->workspace )
   {
      text_to_mobile( dMob, "You have no workspace open.\r\n" );
      return FALSE;
   }
   return TRUE;
}
