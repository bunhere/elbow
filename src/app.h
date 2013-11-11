/*
 * Copyright (C) 2013 Ryuan Choi
 *
 * License LGPL-3, see COPYING file at project folder.
 */
#ifndef app_h
#define app_h

typedef struct _Browser_Data Browser_Data;

struct _Application_Data
{
   Eina_List *browsers;
   Browser_Data *active_browser;

   Ecore_Timer *hide_timer;

   char main_layout_path[256];

   unsigned default_width;
   unsigned default_height;

   Eina_Bool alt_pressed : 1;
   Eina_Bool ctrl_pressed : 1;
   Eina_Bool shift_pressed : 1;
};
typedef struct _Application_Data Application_Data;

#endif // app_h
