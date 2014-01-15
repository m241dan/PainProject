/* File coord.h
   This is the header file for mobile.c
   Written by Davenge */


/* Global Coordinate Variables */
#define MAX_COORD_HASH     1000

extern LIST *coord_map[MAX_COORD_HASH];

struct coordinate
{
   COORD *north;
   COORD *east;
   COORD *south;
   COORD *west;
   COORD *up;
   COORD *down;
   LIST *entities;
   int pos_x;
   int pos_y;
   int pos_z;
};
