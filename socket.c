/*
 * This file contains the socket code, used for accepting
 * new connections as well as reading and writing to
 * sockets, and closing down unused sockets.
 */

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/ioctl.h>
#include <errno.h>

/* including main header file */
#include "mud.h"

/* global variables */
fd_set     fSet;                  /* the socket list for polling       */
STACK    * dsock_free = NULL;     /* the socket free list              */
LIST     * dsock_list = NULL;     /* the linked list of active sockets */
STACK    * dmobile_free = NULL;   /* the mobile free list              */
LIST     * dmobile_list = NULL;   /* the mobile list of active mobiles */
STACK    * account_free = NULL;   /* the account free list -Davenge    */
LIST     * account_list = NULL;   /* the account list of active accounts -Davenge */
LIST     * string_free = NULL;    /* so I can use downcase the way I want -Davenge */
LIST     * coord_map[COORD_HASH_KEY][COORD_HASH_KEY][COORD_HASH_KEY]; /* hash of the coord maps based on the absolute value of X -Davenge */
LIST     * world_entities = NULL; /* a massive list of all entities in the world */
LIST     * id_handlers = NULL;
LIST     * workspaces = NULL;
LIST     * all_frameworks = NULL;
LIST     * all_shapes = NULL;
/* mccp support */
const unsigned char compress_will   [] = { IAC, WILL, TELOPT_COMPRESS,  '\0' };
const unsigned char compress_will2  [] = { IAC, WILL, TELOPT_COMPRESS2, '\0' };
const unsigned char do_echo         [] = { IAC, WONT, TELOPT_ECHO,      '\0' };
const unsigned char dont_echo       [] = { IAC, WILL, TELOPT_ECHO,      '\0' };

/* local procedures */
void GameLoop         ( int control );

/* intialize shutdown state */
bool shut_down = FALSE;
int  control;

/*
 * This is where it all starts, nothing special.
 */
int main(int argc, char **argv)
{
  int x, y, z;
  bool fCopyOver;

  /* get the current time */
  current_time = time(NULL);

  /* allocate memory for socket and mobile lists'n'stacks */
   dsock_free = AllocStack();
   dsock_list = AllocList();
   dmobile_free = AllocStack();
   dmobile_list = AllocList();
   account_free = AllocStack();
   account_list = AllocList();
   string_free = AllocList();
   world_entities = AllocList();
   workspaces = AllocList();
   id_handlers = AllocList();
   all_frameworks = AllocList();
   all_shapes = AllocList();

  /* note that we are booting up */
  log_string("Program starting.");

   log_string( "Initializing the Coordinate Map" );
   for( x = 0; x < COORD_HASH_KEY; x++ )
      for( y = 0; y < COORD_HASH_KEY; y++ )
         for( z = 0; z < COORD_HASH_KEY; z++ )
            coord_map[x][y][z] = AllocList();

   log_string( "Loading ID Handlers" );
   if( !load_id_handlers() )
      exit(EXIT_FAILURE);

   log_string( "Loading Frameworks" );
   if( !load_frameworks() )
      exit(EXIT_FAILURE);

   log_string( "Loading Workspaces" );
   if( !load_workspaces() )
      exit(EXIT_FAILURE);

  /* initialize the event queue - part 1 */
  init_event_queue(1);

  if (argc > 2 && !strcmp(argv[argc-1], "copyover") && atoi(argv[argc-2]) > 0)
  {
    fCopyOver = TRUE;
    control = atoi(argv[argc-2]);
  }
  else fCopyOver = FALSE;

  /* initialize the socket */
  if (!fCopyOver)
    control = init_socket();

  /* load all external data */
  load_muddata(fCopyOver);

  /* initialize the event queue - part 2*/
  init_event_queue(2);

  /* main game loop */
  GameLoop(control);

  /* close down the socket */
  close(control);

  /* terminated without errors */
  log_string("Program terminated without errors.");

  /* and we are done */
  return 0;
}

void GameLoop(int control)   
{
  D_SOCKET *dsock;
  ITERATOR Iter;
  static struct timeval tv;
  struct timeval last_time, new_time;
  extern fd_set fSet;
  fd_set rFd;
  long secs, usecs;

  /* set this for the first loop */
  gettimeofday(&last_time, NULL);

  /* clear out the file socket set */
  FD_ZERO(&fSet);

  /* add control to the set */
  FD_SET(control, &fSet);

  /* copyover recovery */
  AttachIterator(&Iter, dsock_list);
  while ((dsock = (D_SOCKET *) NextInList(&Iter)) != NULL)
    FD_SET(dsock->control, &fSet);
  DetachIterator(&Iter);

  /* do this untill the program is shutdown */
  while (!shut_down)
  {
    /* set current_time */
    current_time = time(NULL);

    /* copy the socket set */
    memcpy(&rFd, &fSet, sizeof(fd_set));

    /* wait for something to happen */
    if (select(FD_SETSIZE, &rFd, NULL, NULL, &tv) < 0)
      continue;

    /* check for new connections */
    if (FD_ISSET(control, &rFd))
    {
      struct sockaddr_in sock;
      unsigned int socksize;
      int newConnection;

      socksize = sizeof(sock);
      if ((newConnection = accept(control, (struct sockaddr*) &sock, &socksize)) >=0)
        new_socket(newConnection);
    }

    /* poll sockets in the socket list */
    AttachIterator(&Iter ,dsock_list);
    while ((dsock = (D_SOCKET *) NextInList(&Iter)) != NULL)
    {
      /*
       * Close sockects we are unable to read from.
       */
      if (FD_ISSET(dsock->control, &rFd) && !read_from_socket(dsock))
      {
        close_socket(dsock, FALSE);
        continue;
      }

      /* Ok, check for a new command */
      next_cmd_from_buffer(dsock);

      /* Is there a new command pending ? */
      if (dsock->next_command[0] != '\0')
      {
        /* figure out how to deal with the incoming command */
        switch(dsock->state)
        {
          default:
            bug("Descriptor in bad state.");
            break;
          case STATE_NEW_NAME:
          case STATE_NEW_PASSWORD:
          case STATE_VERIFY_PASSWORD:
          case STATE_ASK_PASSWORD:
            handle_new_connections(dsock, dsock->next_command);
            break;
          case STATE_PLAYING:
          case STATE_ACCOUNT:
            new_handle_cmd_input( dsock, dsock->next_command );
            break;
          case STATE_NANNY:
             nanny_handle_input( dsock, dsock->next_command );
             break;
        }

        dsock->next_command[0] = '\0';
      }

      /* if the player quits or get's disconnected */
      if (dsock->state == STATE_CLOSED) continue;

      /* Send all new data to the socket and close it if any errors occour */
      if (!flush_output(dsock))
        close_socket(dsock, FALSE);
    }
    DetachIterator(&Iter);

    /* call the event queue */
    heartbeat();

    /*
     * Here we sleep out the rest of the pulse, thus forcing
     * SocketMud(tm) to run at PULSES_PER_SECOND pulses each second.
     */
    gettimeofday(&new_time, NULL);

    /* get the time right now, and calculate how long we should sleep */
    usecs = (int) (last_time.tv_usec -  new_time.tv_usec) + 1000000 / PULSES_PER_SECOND;
    secs  = (int) (last_time.tv_sec  -  new_time.tv_sec);

    /*
     * Now we make sure that 0 <= usecs < 1.000.000
     */
    while (usecs < 0)
    {
      usecs += 1000000;
      secs  -= 1;
    }
    while (usecs >= 1000000)
    {
      usecs -= 1000000;
      secs  += 1;
    }

    /* if secs < 0 we don't sleep, since we have encountered a laghole */
    if (secs > 0 || (secs == 0 && usecs > 0))
    {
      struct timeval sleep_time;

      sleep_time.tv_usec = usecs;
      sleep_time.tv_sec  = secs;

      if (select(0, NULL, NULL, NULL, &sleep_time) < 0)
        continue;
    }

    /* reset the last time we where sleeping */
    gettimeofday(&last_time, NULL);

    /* recycle sockets */
    recycle_sockets();
  }
}

/*
 * Init_socket()
 *
 * Used at bootup to get a free
 * socket to run the server from.
 */
int init_socket()
{
  struct sockaddr_in my_addr;
  int sockfd, reuse = 1;



  /* let's grab a socket */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  /* setting the correct values */
  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = INADDR_ANY;
  my_addr.sin_port = htons(MUDPORT);

  /* this actually fixes any problems with threads */
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1)
  {
    perror("Error in setsockopt()");
    exit(1);
  } 

  /* bind the port */
  bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));

  /* start listening already :) */
  listen(sockfd, 3);

  /* return the socket */
  return sockfd;
}

/* 
 * New_socket()
 *
 * Initializes a new socket, get's the hostname
 * and puts it in the active socket_list.
 */
bool new_socket(int sock)
{
  struct sockaddr_in   sock_addr;
  pthread_attr_t       attr;
  pthread_t            thread_lookup;
  LOOKUP_DATA        * lData;
  D_SOCKET           * sock_new;
  int                  argp = 1;
  socklen_t            size;

  /* initialize threads */
  pthread_attr_init(&attr);   
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  /*
   * allocate some memory for a new socket if
   * there is no free socket in the free_list
   */
  if (StackSize(dsock_free) <= 0)
  {
    if ((sock_new = malloc(sizeof(*sock_new))) == NULL)
    {
      bug("New_socket: Cannot allocate memory for socket.");
      abort();
    }
  }
  else
  {
    sock_new = (D_SOCKET *) PopStack(dsock_free);
  }

  /* attach the new connection to the socket list */
  FD_SET(sock, &fSet);

  /* clear out the socket */
  clear_socket(sock_new, sock);

  /* set the socket as non-blocking */
  ioctl(sock, FIONBIO, &argp);

  /* update the linked list of sockets */
  AttachToList(sock_new, dsock_list);

  /* do a host lookup */
  size = sizeof(sock_addr);
  if (getpeername(sock, (struct sockaddr *) &sock_addr, &size) < 0)
  {
    perror("New_socket: getpeername");
    sock_new->hostname = strdup("unknown");
  }
  else
  {
    /* set the IP number as the temporary hostname */
    sock_new->hostname = strdup(inet_ntoa(sock_addr.sin_addr));

    if (strcasecmp(sock_new->hostname, "127.0.0.1"))
    {
      /* allocate some memory for the lookup data */
      if ((lData = malloc(sizeof(*lData))) == NULL)
      {
        bug("New_socket: Cannot allocate memory for lookup data.");
        abort();
      }

      /* Set the lookup_data for use in lookup_address() */
      lData->buf    =  strdup((char *) &sock_addr.sin_addr);
      lData->dsock  =  sock_new;

      /* dispatch the lookup thread */
      pthread_create(&thread_lookup, &attr, &lookup_address, (void*) lData);
    }
    else sock_new->lookup_status++;
  }

  /* negotiate compression */
  text_to_buffer(sock_new, (char *) compress_will2);
  text_to_buffer(sock_new, (char *) compress_will);

   /* set default page length */
   sock_new->pagewidth = DEFAULT_PAGEWIDTH;

  /* send the greeting */
  text_to_buffer(sock_new, greeting);
  text_to_buffer(sock_new, "What is your name? ");

  /* initialize socket events */
  init_events_socket(sock_new);

  /* everything went as it was supposed to */
  return TRUE;
}

/*
 * Close_socket()
 *
 * Will close one socket directly, freeing all
 * resources and making the socket availably on
 * the socket free_list.
 */
void close_socket(D_SOCKET *dsock, bool reconnect)
{
  EVENT_DATA *pEvent;
  ITERATOR Iter;

  if (dsock->lookup_status > TSTATE_DONE) return;
  dsock->lookup_status += 2;

  /* remove the socket from the polling list */
  FD_CLR(dsock->control, &fSet);

  if (dsock->state == STATE_PLAYING)
  {
    if (reconnect)
      text_to_socket(dsock, "This connection has been taken over.\n\r");
    else if (dsock->entity)
    {
      dsock->entity->socket = NULL;
      log_string("Closing link to %s", dsock->entity->name);
    }
  }
  else if (dsock->entity && dsock->entity->content && dsock->entity->type == MOBILE_ENTITY )
    unload_mobile( (D_MOBILE *)dsock->entity->content);

   if( dsock->account )
      uncontrol_account( dsock->account );

  /* dequeue all events for this socket */
  AttachIterator(&Iter, dsock->events);
  while ((pEvent = (EVENT_DATA *) NextInList(&Iter)) != NULL)
    dequeue_event(pEvent);
  DetachIterator(&Iter);

  /* set the closed state */
  change_socket_state( dsock, STATE_CLOSED );
}

/* 
 * Read_from_socket()
 *
 * Reads one line from the socket, storing it
 * in a buffer for later use. Will also close
 * the socket if it tries a buffer overflow.
 */
bool read_from_socket(D_SOCKET *dsock)
{
  int size;
  extern int errno;

  /* check for buffer overflows, and drop connection in that case */
  size = strlen(dsock->inbuf);
  if (size >= sizeof(dsock->inbuf) - 2)
  {
    text_to_socket(dsock, "\n\r!!!! Input Overflow !!!!\n\r");
    return FALSE;
  }

  /* start reading from the socket */
  for (;;)
  {
    int sInput;
    int wanted = sizeof(dsock->inbuf) - 2 - size;

    sInput = read(dsock->control, dsock->inbuf + size, wanted);

    if (sInput > 0)
    {
      size += sInput;

      if (dsock->inbuf[size-1] == '\n' || dsock->inbuf[size-1] == '\r')
        break;
    }
    else if (sInput == 0)
    {
      log_string("Read_from_socket: EOF");
      return FALSE;
    }
    else if (errno == EAGAIN || sInput == wanted)
      break;
    else
    {
      perror("Read_from_socket");
      return FALSE;
    }     
  }
  dsock->inbuf[size] = '\0';
  return TRUE;
}

/*
 * Text_to_socket()
 *
 * Sends text directly to the socket,
 * will compress the data if needed.
 */
bool text_to_socket(D_SOCKET *dsock, const char *txt)
{
  int iBlck, iPtr, iWrt = 0, length, control = dsock->control;

  length = strlen(txt);

  /* write compressed */
  if (dsock && dsock->out_compress)
  {
    dsock->out_compress->next_in  = (unsigned char *) txt;
    dsock->out_compress->avail_in = length;

    while (dsock->out_compress->avail_in)
    {
      dsock->out_compress->avail_out = COMPRESS_BUF_SIZE - (dsock->out_compress->next_out - dsock->out_compress_buf);

      if (dsock->out_compress->avail_out)
      {
        int status = deflate(dsock->out_compress, Z_SYNC_FLUSH);

        if (status != Z_OK)
        return FALSE;
      }

      length = dsock->out_compress->next_out - dsock->out_compress_buf;
      if (length > 0)
      {
        for (iPtr = 0; iPtr < length; iPtr += iWrt)
        {
          iBlck = UMIN(length - iPtr, 4096);
          if ((iWrt = write(control, dsock->out_compress_buf + iPtr, iBlck)) < 0)
          {
            perror("Text_to_socket (compressed):");
            return FALSE;
          }
        }
        if (iWrt <= 0) break;
        if (iPtr > 0)
        {
          if (iPtr < length)
            memmove(dsock->out_compress_buf, dsock->out_compress_buf + iPtr, length - iPtr);

          dsock->out_compress->next_out = dsock->out_compress_buf + length - iPtr;
        }
      }
    }
    return TRUE;
  }

  /* write uncompressed */
  for (iPtr = 0; iPtr < length; iPtr += iWrt)
  {
    iBlck = UMIN(length - iPtr, 4096);
    if ((iWrt = write(control, txt + iPtr, iBlck)) < 0)
    {
      perror("Text_to_socket:");
      return FALSE;
    }
  }

  return TRUE;
}

/*
 * Text_to_buffer()
 *
 * Stores outbound text in a buffer, where it will
 * stay untill it is flushed in the gameloop.
 *
 * Will also parse ANSI colors and other tags.
 */
void text_to_buffer(D_SOCKET *dsock, const char *txt)
{
  static char output[8 * MAX_BUFFER];
  bool underline = FALSE, bold = FALSE;
  int iPtr = 0, last = -1, j, k;
  int length = strlen( txt );

  /* the color struct */
  struct sAnsiColor
  {
    const char    cTag;
    const char  * cString;
    int           aFlag;
  };

  /* the color table... */
  const struct sAnsiColor ansiTable[] =
  {
    { 'd',  "30",  eTHIN },
    { 'D',  "30",  eBOLD },
    { 'r',  "31",  eTHIN },
    { 'R',  "31",  eBOLD },
    { 'g',  "32",  eTHIN },
    { 'G',  "32",  eBOLD },
    { 'y',  "33",  eTHIN },
    { 'Y',  "33",  eBOLD },
    { 'b',  "34",  eTHIN },
    { 'B',  "34",  eBOLD },
    { 'p',  "35",  eTHIN },
    { 'P',  "35",  eBOLD },
    { 'c',  "36",  eTHIN },
    { 'C',  "36",  eBOLD },
    { 'w',  "37",  eTHIN },
    { 'W',  "37",  eBOLD },

    /* the end tag */
    { '\0',  "",   eTHIN }
  };

  if (length >= MAX_BUFFER)
  {
    log_string("text_to_buffer: buffer overflow.");
    return;
  }

  /* always start with a leading space */
  if (dsock->top_output == 0)
  {
    dsock->outbuf[0] = '\n';
    dsock->outbuf[1] = '\r';
    dsock->top_output = 2;
  }

  while (*txt != '\0')
  {
    /* simple bound checking */
    if (iPtr > (8 * MAX_BUFFER - 15))
      break;

    switch(*txt)
    {
      default:
        output[iPtr++] = *txt++;
        break;
      case '#':
        txt++;

        /* toggle underline on/off with #u */
        if (*txt == 'u')
        {
          txt++;
          if (underline)
          {
            underline = FALSE;
            output[iPtr++] =  27; output[iPtr++] = '['; output[iPtr++] = '0';
            if (bold)
            {
              output[iPtr++] = ';'; output[iPtr++] = '1';
            }
            if (last != -1)
            {
              output[iPtr++] = ';';
              for (j = 0; ansiTable[last].cString[j] != '\0'; j++)
              {
                output[iPtr++] = ansiTable[last].cString[j];
              }
            }
            output[iPtr++] = 'm';
          }
          else
          {
            underline = TRUE;
            output[iPtr++] =  27; output[iPtr++] = '[';
            output[iPtr++] = '4'; output[iPtr++] = 'm';
          }
        }

        /* parse ## to # */
        else if (*txt == '#')
        {
          txt++;
          output[iPtr++] = '#';
        }

        /* #n should clear all tags */
        else if (*txt == 'n')
        {
          txt++;
          if (last != -1 || underline || bold)
          {  
            underline = FALSE;
            bold = FALSE;
            output[iPtr++] =  27; output[iPtr++] = '[';
            output[iPtr++] = '0'; output[iPtr++] = 'm';
          }

          last = -1;
        }

        /* check for valid color tag and parse */
        else
        {
          bool validTag = FALSE;

          for (j = 0; ansiTable[j].cString[0] != '\0'; j++)
          {
            if (*txt == ansiTable[j].cTag)
            {
              validTag = TRUE;

              /* we only add the color sequence if it's needed */
              if (last != j)
              {
                bool cSequence = FALSE;

                /* escape sequence */
                output[iPtr++] = 27; output[iPtr++] = '[';

                /* remember if a color change is needed */
                if (last == -1 || last / 2 != j / 2)
                  cSequence = TRUE;

                /* handle font boldness */
                if (bold && ansiTable[j].aFlag == eTHIN)
                {
                  output[iPtr++] = '0';
                  bold = FALSE;

                  if (underline)
                  {
                    output[iPtr++] = ';'; output[iPtr++] = '4';
                  }

                  /* changing to eTHIN wipes the old color */
                  output[iPtr++] = ';';
                  cSequence = TRUE;
                }
                else if (!bold && ansiTable[j].aFlag == eBOLD)
                {
                  output[iPtr++] = '1';
                  bold = TRUE;

                  if (cSequence)
                    output[iPtr++] = ';';
                }

                /* add color sequence if needed */
                if (cSequence)
                {
                  for (k = 0; ansiTable[j].cString[k] != '\0'; k++)
                  {
                    output[iPtr++] = ansiTable[j].cString[k];
                  }
                }

                output[iPtr++] = 'm';
              }

              /* remember the last color */
              last = j;
            }
          }

          /* it wasn't a valid color tag */
          if (!validTag)
            output[iPtr++] = '#';
          else
            txt++;
        }
        break;   
    }
  }

  /* and terminate it with the standard color */
  if (last != -1 || underline || bold)
  {
    output[iPtr++] =  27; output[iPtr++] = '[';
    output[iPtr++] = '0'; output[iPtr++] = 'm';
  }
  output[iPtr] = '\0';

  /* check to see if the socket can accept that much data */
  if (dsock->top_output + iPtr >= MAX_OUTPUT)
  {
    bug("Text_to_buffer: ouput overflow on %s.", dsock->hostname);
    return;
  }

  /* add data to buffer */
  strcpy(dsock->outbuf + dsock->top_output, output);
  dsock->top_output += iPtr;
}

/*
 * Text_to_mobile()
 *
 * If the mobile has a socket, then the data will
 * be send to text_to_buffer().
 */
void text_to_mobile(D_MOBILE *dMob, const char *txt)
{
   D_SOCKET *dSock = NULL;

   if( dMob->ent_wrapper && dMob->ent_wrapper->socket )
      dSock = dMob->ent_wrapper->socket;

  if( dSock->account )
     txt = handle_pagewidth( dSock->account->pagewidth, txt );
  else
     txt = handle_pagewidth( DEFAULT_PAGEWIDTH, txt );
  text_to_buffer(dSock, txt);
  dSock->bust_prompt = TRUE;
}

void text_to_account( ACCOUNT *account, const char *txt )
{
   if( account->socket )
   {
      txt = handle_pagewidth( account->pagewidth, txt );
      text_to_buffer( account->socket, txt );
      account->socket->bust_prompt = TRUE;
   }
}

void next_cmd_from_buffer(D_SOCKET *dsock)
{
  int size = 0, i = 0, j = 0, telopt = 0;

  /* if theres already a command ready, we return */
  if (dsock->next_command[0] != '\0')
    return;

  /* if there is nothing pending, then return */
  if (dsock->inbuf[0] == '\0')
    return;

  /* check how long the next command is */
  while (dsock->inbuf[size] != '\0' && dsock->inbuf[size] != '\n' && dsock->inbuf[size] != '\r')
    size++;

  /* we only deal with real commands */
  if (dsock->inbuf[size] == '\0')
    return;

  /* copy the next command into next_command */
  for ( ; i < size; i++)
  {
    if (dsock->inbuf[i] == (signed char) IAC)
    {
      telopt = 1;
    }
    else if (telopt == 1 && (dsock->inbuf[i] == (signed char) DO || dsock->inbuf[i] == (signed char) DONT))
    {
      telopt = 2;
    }
    else if (telopt == 2)
    {
      telopt = 0;

      if (dsock->inbuf[i] == (signed char) TELOPT_COMPRESS)         /* check for version 1 */
      {
        if (dsock->inbuf[i-1] == (signed char) DO)                  /* start compressing   */
          compressStart(dsock, TELOPT_COMPRESS);
        else if (dsock->inbuf[i-1] == (signed char) DONT)           /* stop compressing    */
          compressEnd(dsock, TELOPT_COMPRESS, FALSE);
      }
      else if (dsock->inbuf[i] == (signed char) TELOPT_COMPRESS2)   /* check for version 2 */
      {
        if (dsock->inbuf[i-1] == (signed char) DO)                  /* start compressing   */
          compressStart(dsock, TELOPT_COMPRESS2);
        else if (dsock->inbuf[i-1] == (signed char) DONT)           /* stop compressing    */
          compressEnd(dsock, TELOPT_COMPRESS2, FALSE);
      }
    }
    else if (isprint(dsock->inbuf[i]) && isascii(dsock->inbuf[i]))
    {
      dsock->next_command[j++] = dsock->inbuf[i];
    }
  }
  dsock->next_command[j] = '\0';

  /* skip forward to the next line */
  while (dsock->inbuf[size] == '\n' || dsock->inbuf[size] == '\r')
  {
    dsock->bust_prompt = TRUE;   /* seems like a good place to check */
    size++;
  }

  /* use i as a static pointer */
  i = size;

  /* move the context of inbuf down */
  while (dsock->inbuf[size] != '\0')
  {
    dsock->inbuf[size - i] = dsock->inbuf[size];
    size++;
  }
  dsock->inbuf[size - i] = '\0';
}

bool flush_output(D_SOCKET *dsock)
{
  /* nothing to send */
  if (dsock->top_output <= 0 && !(dsock->bust_prompt && dsock->state == STATE_PLAYING))
    return TRUE;

  /* bust a prompt */
   if( dsock->bust_prompt )
   {
      switch( dsock->state )
      {
         case STATE_PLAYING:
            mobile_prompt( dsock );
            break;
         case STATE_ACCOUNT:
            account_prompt( dsock );
            break;
      }
      dsock->bust_prompt = FALSE;
   }

  /* reset the top pointer */
  dsock->top_output = 0;

  /*
   * Send the buffer, and return FALSE
   * if the write fails.
   */
  if (!text_to_socket(dsock, dsock->outbuf))
    return FALSE;

  /* Success */
  return TRUE;
}

void handle_new_connections(D_SOCKET *dsock, char *arg)
{
  ACCOUNT *a_new;
  char aName[MAX_BUFFER];
  int i;

  switch(dsock->state)
  {
    default:
      bug("Handle_new_connections: Bad state.");
      break;
    case STATE_NEW_NAME:
      if (dsock->lookup_status != TSTATE_DONE)
      {
        text_to_buffer(dsock, "Making a dns lookup, please have patience.\n\rWhat is your name? ");
        return;
      }
      if (!check_name(arg)) /* check for a legal name */
      {
        text_to_buffer(dsock, "Sorry, that's not a legal name, please pick another.\n\rWhat is your name? ");
        break;
      }
      log_string("%s is trying to connect.", arg);

      mud_printf( aName, "../accounts/%s/account.afile", capitalize( arg ) ); /* format the file location of where such an account may be located */
      a_new = init_account(); /* initialize a new account structure */
      if( !load_account( aName, a_new ) )/* attempt to load data into it */
      {
        /* give the player it's name */
        a_new->name = strdup(arg);

        /* prepare for next step */
        text_to_buffer(dsock, "Please enter a new password: ");
        dsock->state = STATE_NEW_PASSWORD;
      }
      else /* old player */
      {
        /* prepare for next step */
        text_to_buffer(dsock, "What is your password? ");
        dsock->state = STATE_ASK_PASSWORD;
      }
      text_to_buffer(dsock, (char *) dont_echo);

      control_account( dsock, a_new );
      break;
    case STATE_NEW_PASSWORD:
      if (strlen(arg) < 5 || strlen(arg) > 12)
      {
        text_to_buffer(dsock, "Between 5 and 12 chars please!\n\rPlease enter a new password: ");
        return;
      }
      if( dsock->account->password ) /* just incase */
         free(dsock->account->password);

      dsock->account->password = strdup(crypt(arg, dsock->account->name));

      for (i = 0; dsock->account->password[i] != '\0'; i++)
      {
	if (dsock->account->password[i] == '~')
	{
	  text_to_buffer(dsock, "Illegal password!\n\rPlease enter a new password: ");
	  return;
	}
      }

      text_to_buffer(dsock, "Please verify the password: ");
      dsock->state = STATE_VERIFY_PASSWORD;
      break;
    case STATE_VERIFY_PASSWORD:
      if (!strcmp(crypt(arg, dsock->account->name), dsock->account->password))
      {
        text_to_buffer(dsock, (char *) do_echo);

        /* put him in the list */
        AttachToList(dsock->account, account_list);
        control_account( dsock, dsock->account );

        log_string("New account: %s has entered the game.", dsock->account->name);

        /* and into the game */
        change_socket_state( dsock, STATE_ACCOUNT );

        text_to_buffer(dsock, motd);
        save_account( dsock->account ); /* write the new account */

        /* initialize events on the player */
        /* commented out for now
        init_events_player(dsock->player); */

        /* strip the idle event from this socket */
        strip_event_socket(dsock, EVENT_SOCKET_IDLE);
      }
      else
      {
        if( dsock->account->password )
           free(dsock->account->password);
        dsock->account->password = NULL;
        text_to_buffer(dsock, "Password mismatch!\n\rPlease enter a new password: ");
        dsock->state = STATE_NEW_PASSWORD;
      }
      break;
    case STATE_ASK_PASSWORD:
      text_to_buffer(dsock, (char *) do_echo);
      if (!strcmp(crypt(arg, dsock->account->name), dsock->account->password))
      {
        if ((a_new = check_account_reconnect(dsock->account->name)) != NULL)
        {
          /* kick out whoever is already connected */
          free_account( dsock->account );
          control_account( dsock, a_new );

          log_string("%s has reconnected.", dsock->account->name);

          /* and let him enter the game */
          change_socket_state( dsock, STATE_ACCOUNT );
          text_to_buffer(dsock, "You take over an account already in use.\n\r");

          /* strip the idle event from this socket */
          strip_event_socket(dsock, EVENT_SOCKET_IDLE);
        }
        else
        {
          AttachToList( dsock->account, account_list );
          log_string("%s has entered the game.", dsock->account->name);

          /* and let him enter the game */
          change_socket_state( dsock, STATE_ACCOUNT );
          text_to_buffer(dsock, motd);

	  /* initialize events on the player */
	  /* commented out for now
          init_events_player(dsock->player); */

	  /* strip the idle event from this socket */
	  strip_event_socket(dsock, EVENT_SOCKET_IDLE);
        }
      }
      else
      {
        text_to_socket(dsock, "Bad password!\n\r");
        unload_account( dsock->account );
        dsock->account = NULL;
        close_socket(dsock, FALSE);
      }
      break;
  }
}

void clear_socket(D_SOCKET *sock_new, int sock)
{
  memset(sock_new, 0, sizeof(*sock_new));

  sock_new->control        =  sock;
  sock_new->state          =  STATE_NEW_NAME;
  sock_new->lookup_status  =  TSTATE_LOOKUP;
  sock_new->entity         =  NULL;
  sock_new->top_output     =  0;
  sock_new->events         =  AllocList();
}

/* does the lookup, changes the hostname, and dies */
void *lookup_address(void *arg)
{
  LOOKUP_DATA *lData = (LOOKUP_DATA *) arg;
  struct hostent *from = 0;
  struct hostent ent;
  char buf[16384];
  int err;

  /* do the lookup and store the result at &from */
  gethostbyaddr_r(lData->buf, sizeof(lData->buf), AF_INET, &ent, buf, 16384, &from, &err);

  /* did we get anything ? */
  if (from && from->h_name)
  {
    free(lData->dsock->hostname);
    lData->dsock->hostname = strdup(from->h_name);
  }

  /* set it ready to be closed or used */
  lData->dsock->lookup_status++;

  /* free the lookup data */
  free(lData->buf);
  free(lData);

  /* and kill the thread */
  pthread_exit(0);
}

void recycle_sockets()
{
  D_SOCKET *dsock;
  ITERATOR Iter;

  AttachIterator(&Iter, dsock_list);
  while ((dsock = (D_SOCKET *) NextInList(&Iter)) != NULL)
  {
    if (dsock->lookup_status != TSTATE_CLOSED) continue;

    /* remove the socket from the socket list */
    DetachFromList(dsock, dsock_list);

    /* close the socket */
    close(dsock->control);

    /* free the memory */
    free(dsock->hostname);

    /* free the list of events */
    FreeList(dsock->events);

    /* stop compression */
    compressEnd(dsock, dsock->compressing, TRUE);

    /* put the socket in the free stack */
    PushStack(dsock, dsock_free);
  }
  DetachIterator(&Iter);
}

/* Use this method to change a sockets state -Davenge
 * -Quick note, before changing states,
 *  make sure the proper player/account
 *  is pointed to
 */
void change_socket_state( D_SOCKET *dsock, int state )
{
   dsock->previous_state = dsock->state;
   dsock->state = state;
   switch( state )
   {
      case STATE_PLAYING:
         if( dsock->entity && dsock->entity->commands && dsock->entity->type == MOBILE_ENTITY )
            load_commands( dsock->entity->commands, tabCmd, STATE_PLAYING, ((D_MOBILE *)dsock->entity->content)->level );
         break;
      case STATE_ACCOUNT:
         if( dsock->account && dsock->account->commands )
            load_commands( dsock->account->commands, tabCmd, STATE_ACCOUNT, dsock->account->level );
         break;
   }
   return;
}

/* Set this socket up to control given player */

void control_entity( D_SOCKET *dsock, ENTITY *ent )
{
   if( !dsock )
   {
      bug( "%s: given a NULL socket.", __FUNCTION__ );
      return;
   }
   if( !ent )
   {
      bug( "%s: given a NULL entity.", __FUNCTION__ );
      return;
   }
   dsock->entity = ent;
   ent->socket = dsock;
   return;
}
void uncontrol_entity( ENTITY *ent )
{
   if( !ent )
   {
      bug( "%s: given a NULL ent.", __FUNCTION__ );
      return;
   }
   if( !ent->socket )
   {
      bug( "%s: ent has a NULL socket already.", __FUNCTION__ );
      return;
   }
   ent->socket->entity = NULL;
   ent->socket = NULL;
}

void control_nanny( D_SOCKET *dsock, NANNY *nanny )
{
   if( !dsock )
   {
      bug( "%s: given a NULL socket.", __FUNCTION__ );
      return;
   }
   if( !nanny )
   {
      bug( "%s: given a NULL nanny.", __FUNCTION__ );
      return;
   }
   dsock->nanny = nanny;
   nanny->socket = dsock;
   return;
}

void uncontrol_nanny( NANNY *nanny )
{
   if( !nanny )
   {
      bug( "%s: given a NULL nanny.", __FUNCTION__ );
      return;
   }
   if( !nanny->socket )
   {
      bug( "%s: given a nanny with a NULL socket.", __FUNCTION__ );
      return;
   }
   nanny->socket->nanny = NULL;
   nanny->socket = NULL;
}

void control_account( D_SOCKET *dsock, ACCOUNT *account )
{
   if( !dsock )
   {
      bug( "%s: given a NULL socket.", __FUNCTION__ );
      return;
   }
   if( !account )
   {
      bug( "%s: given a NULL account.", __FUNCTION__ );
      return;
   }
   dsock->account = account;
   account->socket = dsock;
   return;
}

void uncontrol_account( ACCOUNT *account )
{
   if( !account )
   {
      bug( "%s: given a NULL account.", __FUNCTION__ );
      return;
   }
   if( !account->socket )
   {
      bug( "%s: given an account with a NULL socket.", __FUNCTION__ );
      return;
   }
   account->socket->account = NULL;
   account->socket = NULL;
}
const char *handle_pagewidth( int width, const char *txt )
{
   static char buf[MAX_BUFFER * 2];
   bool color = FALSE;
   char *ptr;
   int x;

   memset( &buf[0], 0, sizeof(buf) );
   ptr = buf;
   x = 0;

   while( *txt != '\0' )
   {
      if( *txt == '#' && !color )
         color = TRUE;
      else if( *txt == '#' && color )
      {
         x++;
         color = FALSE;
      }

      if( *txt != '#' )
      {
         if( color )
            color = FALSE;
         else
            x++;
      }

      if( *txt == '\n' || *txt == '\r' )
         x = 1;

      if( x > width )
      {
         x = 1;
         *ptr++ = '\n';
         *ptr++ = '\r';
      }
      *ptr++ = *txt++;
   }

   buf[strlen(buf)] = '\0';

   return buf;
}
