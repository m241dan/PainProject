/*
 * This is the main headerfile
 */

#ifndef MUD_H
#define MUD_H

#include <zlib.h>
#include <pthread.h>
#include <arpa/telnet.h>
#include "list.h"
#include "stack.h"

/************************
 * Standard definitions *
 ************************/

/* define TRUE and FALSE */
#ifndef FALSE
#define FALSE   0
#endif
#ifndef TRUE
#define TRUE    1
#endif

#define eTHIN   0
#define eBOLD   1

/* A few globals */
#define PULSES_PER_SECOND     4                   /* must divide 1000 : 4, 5 or 8 works */
#define MAX_BUFFER         1024                   /* seems like a decent amount         */
#define MAX_OUTPUT         2048                   /* well shoot me if it isn't enough   */
#define MAX_HELP_ENTRY     4096                   /* roughly 40 lines of blocktext      */
#define MUDPORT            6500                   /* just set whatever port you want    */
#define FILE_TERMINATOR    "EOF"                  /* end of file marker                 */
#define COPYOVER_FILE      "../txt/copyover.dat"  /* tempfile to store copyover data    */
#define EXE_FILE           "../src/SocketMud"     /* the name of the mud binary         */

/* Connection states */
typedef enum {
   /* Account Creation States */
   STATE_NEW_NAME, STATE_NEW_PASSWORD, STATE_VERIFY_PASSWORD, STATE_ASK_PASSWORD, 
   /* Account States */
   STATE_ACCOUNT,
   /* Character Creation States */

   /* Playing States */
   STATE_PLAYING,
   /* Other */
   STATE_CLOSED, MAX_STATE
} socket_states;

/* Thread states - please do not change the order of these states    */
#define TSTATE_LOOKUP          0  /* Socket is in host_lookup        */
#define TSTATE_DONE            1  /* The lookup is done.             */
#define TSTATE_WAIT            2  /* Closed while in thread.         */
#define TSTATE_CLOSED          3  /* Closed, ready to be recycled.   */

/* player levels */
#define LEVEL_GUEST            1  /* Dead players and actual guests  */
#define LEVEL_PLAYER           2  /* Almost everyone is this level   */
#define LEVEL_ADMIN            3  /* Any admin without shell access  */
#define LEVEL_GOD              4  /* Any admin with shell access     */

/* Communication Ranges */
#define COMM_LOCAL             0  /* same room only                  */
#define COMM_LOG              10  /* admins only                     */

/* define simple types */
typedef  unsigned char     bool;
typedef  short int         sh_int;

/******************************
 * End of standard definitons *
 ******************************/

/***********************
 * Defintion of Macros *
 ***********************/

#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define IS_ADMIN(dMob)          ((dMob->level) > LEVEL_PLAYER ? TRUE : FALSE)
#define IREAD(sKey, sPtr)             \
{                                     \
  if (!strcasecmp(sKey, word))        \
  {                                   \
    int sValue = fread_number(fp);    \
    sPtr = sValue;                    \
    found = TRUE;                     \
    break;                            \
  }                                   \
}
#define SREAD(sKey, sPtr)             \
{                                     \
  if (!strcasecmp(sKey, word))        \
  {                                   \
    sPtr = fread_string(fp);          \
    found = TRUE;                     \
    break;                            \
  }                                   \
}
/*
 * A memory allocation macro, because its what I'm used to using
 * Written by Davenge
 */
#define CREATE(result, type, number)                                    \
do                                                                      \
{                                                                       \
   if (!((result) = (type *) calloc ((number), sizeof(type))))          \
   {                                                                    \
      perror("malloc failure");                                         \
      fprintf(stderr, "Malloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
      abort();                                                          \
   }                                                                    \
} while(0)


/***********************
 * End of Macros       *
 ***********************/

/******************************
 * New structures             *
 ******************************/

/* type defintions */
typedef struct  dSocket       D_SOCKET;
typedef struct  dMobile       D_MOBILE;
typedef struct  help_data     HELP_DATA;
typedef struct  lookup_data   LOOKUP_DATA;
typedef struct  event_data    EVENT_DATA;
typedef struct game_account ACCOUNT;
typedef struct typCmd COMMAND;

/* the actual structures */
struct dSocket
{
  D_MOBILE      * player;
  LIST          * events;
  char          * hostname;
  char            inbuf[MAX_BUFFER];
  char            outbuf[MAX_OUTPUT];
  char            next_command[MAX_BUFFER];
  bool            bust_prompt;
  sh_int          lookup_status;
  sh_int          state;
  sh_int          control;
  sh_int          top_output;
  unsigned char   compressing;                 /* MCCP support */
  z_stream      * out_compress;                /* MCCP support */
  unsigned char * out_compress_buf;            /* MCCP support */

  /* New Stuff */
  ACCOUNT       * account; /* sockets now hold accounts */
};

struct dMobile
{
  D_SOCKET      * socket;
  LIST          * events;
  char          * name;
  char          * password;
  sh_int          level;
};

struct help_data
{
  time_t          load_time;
  char          * keyword;
  char          * text;
};

struct lookup_data
{
  D_SOCKET       * dsock;   /* the socket we wish to do a hostlookup on */
  char           * buf;     /* the buffer it should be stored in        */
};

struct typCmd
{
  char      * cmd_name;
  void     (* cmd_funct)(void *passed, char *arg);
  sh_int    level;
  sh_int    state;
};

typedef struct buffer_type
{
  char   * data;        /* The data                      */
  int      len;         /* The current len of the buffer */
  int      size;        /* The allocated size of data    */
} BUFFER;


/* here we include external structure headers */
#include "event.h"
#include "account.h"
#include "nanny.h"

/******************************
 * End of new structures      *
 ******************************/

/***************************
 * Global Variables        *
 ***************************/

extern  STACK       *   dsock_free;       /* the socket free list               */
extern  LIST        *   dsock_list;       /* the linked list of active sockets  */
extern  STACK       *   dmobile_free;     /* the mobile free list               */
extern  LIST        *   dmobile_list;     /* the mobile list of active mobiles  */
extern  LIST        *   help_list;        /* the linked list of help files      */
extern  STACK       *   account_free;     /* list of free accounts -Davenge     */
extern  LIST        *   account_list;     /* list of active accounts -Davenge   */
extern  const struct    typCmd tabCmd[];  /* the command table                  */
extern  bool            shut_down;        /* used for shutdown                  */
extern  char        *   greeting;         /* the welcome greeting               */
extern  char        *   motd;             /* the MOTD help file                 */
extern  int             control;          /* boot control socket thingy         */
extern  time_t          current_time;     /* let's cut down on calls to time()  */

/*************************** 
 * End of Global Variables *
 ***************************/

/***********************
 *    MCCP support     *
 ***********************/

extern const unsigned char compress_will[];
extern const unsigned char compress_will2[];

#define TELOPT_COMPRESS       85
#define TELOPT_COMPRESS2      86
#define COMPRESS_BUF_SIZE   8192

/***********************
 * End of MCCP support *
 ***********************/

/***********************************
 * Prototype function declerations *
 ***********************************/

/* more compact */
#define  D_S         D_SOCKET
#define  D_M         D_MOBILE

#define  buffer_new(size)             __buffer_new     ( size)
#define  buffer_strcat(buffer,text)   __buffer_strcat  ( buffer, text )

char  *crypt                  ( const char *key, const char *salt );

/*
 * socket.c
 */
int   init_socket             ( void );
bool  new_socket              ( int sock );
void  close_socket            ( D_S *dsock, bool reconnect );
bool  read_from_socket        ( D_S *dsock );
bool  text_to_socket          ( D_S *dsock, const char *txt );  /* sends the output directly */
void  text_to_buffer          ( D_S *dsock, const char *txt );  /* buffers the output        */
void  text_to_mobile          ( D_M *dMob, const char *txt );   /* buffers the output        */
void  next_cmd_from_buffer    ( D_S *dsock );
bool  flush_output            ( D_S *dsock );
void  handle_new_connections  ( D_S *dsock, char *arg );
void  clear_socket            ( D_S *sock_new, int sock );
void  recycle_sockets         ( void );
void *lookup_address          ( void *arg );

/*
 * interpret.c
 */
void  handle_cmd_input        ( D_S *dsock, char *arg );
void new_handle_cmd_input ( D_S *dsock, char *arg );
/*
 * io.c
 */
void    log_string            ( const char *txt, ... );
void    bug                   ( const char *txt, ... );
time_t  last_modified         ( char *helpfile );
char   *read_help_entry       ( const char *helpfile );     /* pointer         */
char   *fread_line            ( FILE *fp );                 /* pointer         */
char   *fread_string          ( FILE *fp );                 /* allocated data  */
char   *fread_word            ( FILE *fp );                 /* pointer         */
int     fread_number          ( FILE *fp );                 /* just an integer */

/* 
 * strings.c
 */
char   *one_arg               ( char *fStr, char *bStr );
char   *strdup                ( const char *s );
int     strcasecmp            ( const char *s1, const char *s2 );
bool    is_prefix             ( const char *aStr, const char *bStr );
char   *capitalize_word       ( char *txt ); /* changed make my own styles of capitalize methods -Davenge */
BUFFER *__buffer_new          ( int size );
void    __buffer_strcat       ( BUFFER *buffer, const char *text );
void    buffer_free           ( BUFFER *buffer );
void    buffer_clear          ( BUFFER *buffer );
int     bprintf               ( BUFFER *buffer, char *fmt, ... );
char   *downcase	      ( const char *word );
char   *capitalize            ( const char *word );
bool   downcase_orig          ( char *word );
bool   capitalize_orig        ( char *word );
void   spit_equals            ( D_SOCKET *dsock, int amount );
char   *produce_equals        ( int amount );
/*
 * help.c
 */
bool  check_help              ( D_M *dMob, char *helpfile );
void  load_helps              ( void );

/*
 * utils.c
 */
bool  check_name              ( const char *name );
void  clear_mobile            ( D_M *dMob );
void  free_mobile             ( D_M *dMob );
void  communicate             ( D_M *dMob, char *txt, int range );
void  load_muddata            ( bool fCopyOver );
char *get_time                ( void );
void  copyover_recover        ( void );
D_M  *check_reconnect         ( char *player );
COMMAND *copy_command( const struct typCmd to_copy );
void free_command( COMMAND *command );
/*
 * action_safe.c
 */
void  cmd_say                 ( void *passed, char *arg );
void  cmd_quit                ( void *passed, char *arg );
void  cmd_shutdown            ( void *passed, char *arg );
void  cmd_commands            ( void *passed, char *arg );
void  cmd_who                 ( void *passed, char *arg );
void  cmd_help                ( void *passed, char *arg );
void  cmd_compress            ( void *passed, char *arg );
void  cmd_save                ( void *passed, char *arg );
void  cmd_copyover            ( void *passed, char *arg );
void  cmd_linkdead            ( void *passed, char *arg );

/*
 * mccp.c
 */
bool  compressStart           ( D_S *dsock, unsigned char teleopt );
bool  compressEnd             ( D_S *dsock, unsigned char teleopt, bool forced );

/*
 * save.c
 */
void  save_player             ( D_M *dMob );
D_M  *load_player             ( char *player );
D_M  *load_profile            ( char *player );

/*******************************
 * End of prototype declartion *
 *******************************/

#endif  /* MUD_H */
