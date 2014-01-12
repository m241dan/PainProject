#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* main header file */
#include "mud.h"

void save_pfile           ( D_MOBILE *dMob );
void save_profile         ( D_MOBILE *dMob );

void save_pfile(D_MOBILE *dMob)
{
  char pName[MAX_BUFFER];
  char pfile[MAX_BUFFER];
  FILE *fp;
  int size, i;

  pName[0] = toupper(dMob->name[0]);
  size = strlen(dMob->name);
  for (i = 1; i < size && i < MAX_BUFFER - 1; i++)
    pName[i] = tolower(dMob->name[i]);
  pName[i] = '\0';

  /* open the pfile so we can write to it */
  snprintf(pfile, MAX_BUFFER, "../players/%s.pfile", pName);
  if ((fp = fopen(pfile, "w")) == NULL)
  {
    bug("Unable to write to %s's pfile", dMob->name);
    return;
  }

  /* dump the players data into the file */
  fprintf(fp, "Name            %s~\n", dMob->name);
  fprintf(fp, "Level           %d\n",  dMob->level);
  fprintf(fp, "Password        %s~\n", dMob->password);
  fprintf(fp, "Race            %d\n",  dMob->race);

  /* terminate the file */
  fprintf(fp, "%s\n", FILE_TERMINATOR);
  fclose(fp);
}

/*
 * This file stores only data vital to load
 * the character, and check for things like
 * password and other such data.
 */
void save_profile(D_MOBILE *dMob)
{
  char pfile[MAX_BUFFER];
  char pName[MAX_BUFFER];
  FILE *fp;
  int size, i;
  
  pName[0] = toupper(dMob->name[0]);
  size = strlen(dMob->name);
  for (i = 1; i < size && i < MAX_BUFFER - 1; i++)
    pName[i] = tolower(dMob->name[i]);
  pName[i] = '\0';
  
  /* open the pfile so we can write to it */
  snprintf(pfile, MAX_BUFFER, "../players/%s.profile", pName);
  if ((fp = fopen(pfile, "w")) == NULL)
  {
    bug("Unable to write to %s's pfile", dMob->name);
    return;
  }

  /* dump the players data into the file */
  fprintf(fp, "Name           %s~\n", dMob->name);
  fprintf(fp, "Password       %s~\n", dMob->password);

  /* terminate the file */
  fprintf(fp, "%s\n", FILE_TERMINATOR);
  fclose(fp);
}
