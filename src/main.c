/*
 * Copyright (C) 2013 Ryuan Choi
 */

#include "app.h"
#include "browser.h"
#include "database/bookmark.h"
#include "log.h"
#include "webview.h"
#include <EWebKit2.h>
#include <Elementary.h>

static Eina_Bool
main_key_down_cb(void *data, int type EINA_UNUSED, void *ev)
{
   Application_Data *ad = data;
   Ecore_Event_Key *event = ev;
   const char *keyname = event->keyname;

   if (!strcmp("Control_L", keyname))
     ad->ctrl_pressed = EINA_TRUE;
   if (!strcmp("Alt_L", keyname))
     ad->alt_pressed = EINA_TRUE;
   if (!strcmp("Shift_L", keyname))
     ad->shift_pressed = EINA_TRUE;

   return browser_keydown(ad->active_browser, keyname, ad->ctrl_pressed, ad->alt_pressed, ad->shift_pressed);
}

static Eina_Bool
main_key_up_cb(void *data, int type EINA_UNUSED, void *ev)
{
   Ecore_Event_Key *event = ev;
   Application_Data *ad = data;

   if (!strcmp("Control_L", event->keyname))
     ad->ctrl_pressed = EINA_FALSE;
   else if (!strcmp("Shift_L", event->keyname))
     ad->shift_pressed = EINA_FALSE;
   else if (!strcmp("Alt_L", event->keyname))
     {
        ad->alt_pressed = EINA_FALSE;
     }

   return ECORE_CALLBACK_PASS_ON;
}

EAPI_MAIN int
elm_main(int argc, char** argv)
{
   BROWSER_LOGD("%s", "h");
   Browser_Data *bd;
   Application_Data ad;

   ewk_init();

   memset(&ad, 0x00, sizeof(ad));

   //FIXME : Check installation path(for release) with build directory(for development)
   strcpy(ad.main_layout_path, THEME_BUILD_PATH "/elbow.edj");
   ad.default_width = DEFAULT_WIDTH;
   ad.default_height = DEFAULT_HEIGHT;

   ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, main_key_down_cb, &ad);
   ecore_event_handler_add(ECORE_EVENT_KEY_UP, main_key_up_cb, &ad);

   bd = browser_add(&ad, application_default_url(&ad));
   ad.active_browser = bd;

   evas_object_resize(bd->win, ad.default_width, ad.default_height);
   evas_object_show(bd->win);

   elm_run();

   ewk_shutdown();

   bookmark_shutdown();

   elm_shutdown();

   return 0;
}
ELM_MAIN()
