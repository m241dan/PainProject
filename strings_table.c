/* File - strings_table.c
 * Strings table, this is where you can find tables full of strings for things and stuff...
 * Written by Davenge */

#include <stdio.h>
#include "mud.h"

const char *get_table( const char *const string_table[], int size )
{
   static char buf[MAX_BUFFER];
   char buf2[MAX_BUFFER];
   int x;

   buf[0] = '\0';
   for( x = 0; x < size; x++ )
   {
      snprintf( buf2, MAX_BUFFER, "%s ", string_table[x] );
      strcat( buf, buf2 );
   }

   buf[strlen(buf)] = '\0';
   return buf;
}

const char *const nanny_strings[MAX_NANNY_TYPE][MAX_NANNY_STATES] = {
   { "What would you like to name this character?", "(Optional)Enter an Additional Password(blank space for none): ", "Retype Password to Confirm: ", "Please type in your race: " } /* type 1 Nanny */
};

const char *const race_table[MAX_RACE] = {
   "Human", "Saiyan", "Halfbreed", "Namek", "Icer", "Android", "BioAndroid", "Majin", "Demon", "Kaio", "Tuffle", "Dragon"
};

const char *const race_desc_table[MAX_RACE] = {
   "All Around Balance",
   "Melee+Ki Offense/Afflictions",
   "Weaker All Around Balance/Gains",
   "Ki Offense+Defense/Healing",
   "All Power/High Mods",
   "Tech shit?",
   "Strong Ki/Poor Melee",
   "Afflictions/Healing",
   "Afflictions/Melee+Ki",
   "Barrier/Healer",
   "Tech Buffs/Debuffs",
   "All Power/High Mods",
};
