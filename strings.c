/*
 * This file handles string copy/search/comparison/etc.
 */
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

/* include main header file */
#include "mud.h"

/*
 * Checks if aStr is a prefix of bStr.
 */
bool is_prefix(const char *aStr, const char *bStr)
{
  /* NULL strings never compares */
  if (aStr == NULL || bStr == NULL) return FALSE;

  /* empty strings never compares */
  if (aStr[0] == '\0' || bStr[0] == '\0') return FALSE;

  /* check if aStr is a prefix of bStr */
  while (*aStr)
  {
    if (tolower(*aStr++) != tolower(*bStr++))
      return FALSE;
  }

  /* success */
  return TRUE;
}

char *one_arg(char *fStr, char *bStr)
{
  /* skip leading spaces */
  while (isspace(*fStr))
    fStr++; 

  /* copy the beginning of the string */
  while (*fStr != '\0')
  {
    /* have we reached the end of the first word ? */
    if (*fStr == ' ')
    {
      fStr++;
      break;
    }

    /* copy one char */
    *bStr++ = *fStr++;
  }

  /* terminate string */
  *bStr = '\0';

  /* skip past any leftover spaces */
  while (isspace(*fStr))
    fStr++;

  /* return the leftovers */
  return fStr;
}

char *capitalize_word(char *txt)
{
  static char buf[MAX_BUFFER];
  int size, i;

  buf[0] = '\0';

  if (txt == NULL || txt[0] == '\0')
    return buf;

  size = strlen(txt);

  for (i = 0; i < size; i++)
    buf[i] = toupper(txt[i]);
  buf[size] = '\0';

  return buf;
}

/*  
 * Create a new buffer.
 */
BUFFER *__buffer_new(int size)
{
  BUFFER *buffer;
    
  buffer = malloc(sizeof(BUFFER));
  buffer->size = size;
  buffer->data = malloc(size);
  buffer->len = 0;
  return buffer;
}

/*
 * Add a string to a buffer. Expand if necessary
 */
void __buffer_strcat(BUFFER *buffer, const char *text)  
{
  int new_size;
  int text_len;
  char *new_data;
 
  /* Adding NULL string ? */
  if (!text)
    return;

  text_len = strlen(text);
    
  /* Adding empty string ? */ 
  if (text_len == 0)
    return;

  /* Will the combined len of the added text and the current text exceed our buffer? */
  if ((text_len + buffer->len + 1) > buffer->size)
  { 
    new_size = buffer->size + text_len + 1;
   
    /* Allocate the new buffer */
    new_data = malloc(new_size);
  
    /* Copy the current buffer to the new buffer */
    memcpy(new_data, buffer->data, buffer->len);
    free(buffer->data);
    buffer->data = new_data;  
    buffer->size = new_size;
  }
  memcpy(buffer->data + buffer->len, text, text_len);
  buffer->len += text_len;
  buffer->data[buffer->len] = '\0';
}

/* free a buffer */
void buffer_free(BUFFER *buffer)
{
  /* Free data */
  free(buffer->data);
 
  /* Free buffer */
  free(buffer);
}

/* Clear a buffer's contents, but do not deallocate anything */
void buffer_clear(BUFFER *buffer)
{
  buffer->len = 0;
  buffer->data[0] = '\0';
}

/* print stuff, append to buffer. safe. */
int bprintf(BUFFER *buffer, char *fmt, ...)
{  
  char buf[MAX_BUFFER];
  va_list va;
  int res;
    
  va_start(va, fmt);
  res = vsnprintf(buf, MAX_BUFFER, fmt, va);
  va_end(va);
    
  if (res >= MAX_BUFFER - 1)  
  {
    buf[0] = '\0';
    bug("Overflow when printing string %s", fmt);
  }
  else
    buffer_strcat(buffer, buf);
   
  return res;
}

char *strdup(const char *s)
{
  char *pstr;
  int len;

  len = strlen(s) + 1;
  pstr = (char *) calloc(1, len);
  strcpy(pstr, s);

  return pstr;
}

int strcasecmp(const char *s1, const char *s2)
{
  int i = 0;

  while (s1[i] != '\0' && s2[i] != '\0' && toupper(s1[i]) == toupper(s2[i]))
    i++;

  /* if they matched, return 0 */
  if (s1[i] == '\0' && s2[i] == '\0')
    return 0;

  /* is s1 a prefix of s2? */
  if (s1[i] == '\0')
    return -110;

  /* is s2 a prefix of s1? */
  if (s2[i] == '\0')
    return 110;

  /* is s1 less than s2? */
  if (toupper(s1[i]) < toupper(s2[i]))
    return -1;

  /* s2 is less than s1 */
  return 1;
}

/*
 * Downcase and Capitalize that return a pointer to a new string
 * Written by Davenge
 */
/* The problem with this function is you can't compare two downcased strings because its a static buf but otherwise you have to make new variables etc */
char *downcase( const char *word )
{
   static char buf[MAX_BUFFER]; /* has to be buffer or compiler throws error, the it also has to be static so there are no memory leaks */
   int length, x;


   if( !word || word[0] == '\0' ) /* make sure we are working with something that isn't null */
      return buf;

   length = strlen( word ); /* get the length of the word so we know how far to iterate */

   for( x = 0; x < length; x++ )
      buf[x] = tolower( word[x] ); /* iterate through the word and set the lowercase version of each letter into the buf */
   buf[x] = '\0'; /* make sure to give our new string a null terminator */

   return buf; /* return it */
}


/*
 * Most of these operation are the same, except we capitalize the first letter
 * -Davenge
 */
char *capitalize( const char *word )
{
   static char buf[MAX_BUFFER];
   char *dWord;
   int length, x;
   buf[0] = '\0';

   if( !word || word[0] == '\0')
      return buf;

   dWord = downcase( word );
   length = strlen( dWord );

   for( x = 0; x < length; x++ ) /* copy the downcased string into our buffer... there's gotta be a better way */
      buf[x] = dWord[x];

   buf[0] = toupper( buf[0] ); /* Capitalize the first letter */

   buf[strlen(word)] = '\0';

   return buf;
}

/* downcase_orig and capitalize_orig that modify the string passed
 * Written by Davenge
 */

bool downcase_orig( char * word )
{
   int x, length = strlen( word );

   if( !word || word[0] == '\0' )
      return FALSE; /* return false because string passed as unmodifiable */

   for( x = 0; x < length; x++ )
      word[x] = tolower( word[x] );

   return TRUE; /* successfully modified */

}

bool capitalize_orig( char * word )
{
   if( !word || word[0] == '\0' )
      return FALSE;

   downcase_orig( word ); /* make it all lowercase first */
   word[0] = toupper( word[0] ); /* capitalize the first letter, like a proper noun */

   return TRUE;
}

/* some output stuff for pretty formatting
 * Written by Davenge
 */
char *smash_color( const char *str )
{
   static char ret[MAX_BUFFER];
   char *retptr;

   retptr = ret;

   if(str == NULL)
      return NULL;

   for ( ; *str != '\0'; str++ )
   {
      if (*str == '#' && *(str + 1) != '\0' )
         str++;
      else
      {
         *retptr = *str;

         retptr++;
      }
   }
   *retptr = '\0';
   return ret;
}

void spit_equals( D_SOCKET *dsock, int amount )
{
   int x;

   for( x = 0; x < amount; x++ )
      text_to_buffer( dsock, "=" );

   return;
}

char *produce_equals( int amount )
{
   static char equals[MAX_BUFFER];
   char *ptr;
   int x;
   ptr = equals;
   equals[amount] = '\0';

   for( x = 0; x < amount; x++, ptr++ )
      *ptr = '=';

   return equals;
}
