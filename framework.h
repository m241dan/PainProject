/* File framework.h
   This is the header file for framework.c
   Written by Davenge */


typedef enum {
   FRAME_ROOM, MAX_FRAME
} framework_type_enum;


struct framework
{
   void *content;
   int type;
};

struct room_framework
{
   char *title;
   char *description;
};
