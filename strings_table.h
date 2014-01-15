/* File strings_table.h
 * Here is the Header File for the Strings Table
 * Written by Davenge */

/* Definitions */
const char *const entity_types[MAX_ENTITY+1];
extern const char *const framework_types[MAX_FRAME+1];
extern const char *const nanny_strings[MAX_NANNY_TYPE][MAX_NANNY_STATES];
extern const char *const race_table[];
extern const char *const race_desc_table[];
const char *const exit_directions[MAX_DIRECTION+1];

/* Methods */
/* General */
int match_string_table( const char *string, const char *const string_table[] );
const char *get_table( const char *const string_table[] );
int str_table_max_strlen( const char *const table[] );
void fit_space_with_color( char *dest, const char *orig, int space );
/* Table Specific */
void show_race_table( D_SOCKET *dsock );
