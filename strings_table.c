/* File - strings_table.c
 * Strings table, this is where you can find tables full of strings for things and stuff...
 * Written by Davenge */

#include <stdio.h>
#include "mud.h"

const char *const nanny_strings[MAX_NANNY_TYPE][MAX_NANNY_STATES] = {
   { "What would you like to name this character?", "(Optional)Enter an Additional Password(blank space for none): ", "Retype Password to Confirm: " } /* type 1 Nanny */
};
