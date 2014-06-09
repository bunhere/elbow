/*
 * Copyright (C) 2013-2014 Ryuan Choi
 */

#include "homescreen.h"

#include "app.h"
#include "browser.h"
#include "log.h"
#include <Elementary.h>

struct _HomeScreen {
   Evas_Object *layout;
};
typedef struct _HomeScreen HomeScreen;

static void
_home_screen_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   BROWSER_CALL_LOG("");
   free(data);
}

Evas_Object *
home_screen_add(Browser_Data *bd)
{
   BROWSER_CALL_LOG("");

   HomeScreen *screen;
   Evas_Object *layout;

   screen = (HomeScreen *)malloc(sizeof(HomeScreen));
   if (!screen) return NULL;

   screen->layout = layout = elm_layout_add(bd->win);
   evas_object_data_set(layout, "_container", bd);
   elm_layout_file_set(layout, bd->ad->main_layout_path, "home_screen");

   evas_object_event_callback_add(layout, EVAS_CALLBACK_DEL, _home_screen_del, screen);

   evas_object_data_set(layout, "_self", screen);

   return layout;
}
