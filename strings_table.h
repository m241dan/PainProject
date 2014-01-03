/* File strings_table.h
 * Here is the Header File for the Strings Table
 * Written by Davenge */

/* Definitions */
extern const char *const nanny_strings[MAX_NANNY_TYPE][MAX_NANNY_STATES];
extern const char *const race_table[];
extern const char *const race_desc_table[];

/* Methods */
/* General */
const char *get_table( const char *const string_table[] );
int str_table_max_strlen( const char *const table[] );
void fit_space_with_color( char *dest, const char *orig, int space );
/* Table Specific */
void show_race_table( D_SOCKET *dsock );
