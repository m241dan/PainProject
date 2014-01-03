/* File - strings_table.c
 * Strings table, this is where you can find tables full of strings for things and stuff...
 * Written by Davenge */

/**************************************************
 * Please terminate all string tables with a ""   *
 * Please make sure all string tables are nolimit *
 * (Unless it is a two dimensional table)         *
 * Thanks, Davenge                                *
 **************************************************/

#include <stdio.h>
#include "mud.h"

const char *get_table( const char *const string_table[] )
{
   static char buf[MAX_BUFFER];
   int x;

   buf[0] = '\0';
   for( x = 0; string_table[x][0] != '\0'; x++ )
      strcat( buf, string_table[x] ); 
   buf[strlen(buf)] = '\0';
   return buf;
}

const char *const nanny_strings[MAX_NANNY_TYPE][MAX_NANNY_STATES] = {
   { "What would you like to name this character?", "(Optional)Enter an Additional Password(blank space for none): ", "Retype Password to Confirm: ", "Please type in your race: ", "" }, /* type 1 Nanny */
   /* The terminator */
   { "" }
};

const char *const race_table[] = {
   "Human", "Saiyan", "Halfbreed", "Namek", "Icer", "Android", "BioAndroid", "Majin", "Demon", "Kaio", "Tuffle", "Dragon", ""
};

const char *const race_desc_table[] = {
   "#WWell Rounded#D/#WDecent Mods#D/#WGood Gains#D",
   "#WOffensive Melee+Ki#D/#WGood Mods#D/#WGood Gains#D",
   "#WLow Rounded#D/#WLow Mods#D/#WGreat Gains#D",
   "#WOffensive Ki#D/Healing#W/#DAverage Mods#W/#DGood Gains",
   "#WStrong Offensive Ki#D/#WGood Mods#D/#WLow Gains#D",
   "#WStrong Offensive Melee+Ki#D/#WAverage Mods#D/#WLow Gains#D",
   "#WStrong Rounded#D/#WGood Mods#D/#WLow Gains#D",
   "#WStrong Ki#D/#WGood Mods#D/#WDecent Gains#D",
   "#WStrong Rounded#D/#WGood Mods#D/#WLow Gains#D",
   "#WRounded#D/#WStrong Healing#D/#WLow Mods#D/#WGood Gains#D",
   "#WLow Rounded#D/#WBuffs+Debuffs#D/#WLow Mods#D/#WDecent Gains#D",
   "#WStrong Rounded#D/#WBuffs#D/#WHigh Mods#D/#WVery Low Gains#D",
   "",
};

void show_race_table( D_SOCKET *dsock )
{
   BUFFER *buf = buffer_new( MAX_BUFFER );
   int namelen, desclen, x;

   /* get the max length of any string in the name and desc table */
   namelen = str_table_max_strlen( race_table );
   desclen = str_table_max_strlen( race_desc_table );

   /* iterate the arrays and put our data into the buffer */
   for( x = 0; race_table[x][0] != '\0' || race_desc_table[x][0] != '\0'; x++ )
      

   return;

}

int str_table_max_strlen( const char *const table[] )
{
   int x, j, length;
   for( x = 0, length = 0; table[x][0] != '\0'; x++ )
      if( ( j = strlen( table[x] ) ) > length )
         length = j;
   return length;
}
