/*
 * Copyright (C) 2013 Ryuan Choi
 *
 * License LGPL-3, see COPYING file at project folder.
 */

#include <Elementary.h>
#include "app.h"
#include "browser.h"
#include "webview.h"

#if defined(USE_EWEBKIT)
#include <EWebKit.h>
#elif defined(USE_EWEBKIT2)
#include <EWebKit2.h>
#endif

static Eina_Bool
_hide_menu_cb(void *data)
{
   Application_Data *ad = data;
   browser_urlbar_hide(ad->active_browser);
   browser_multiplebar_hide(ad->active_browser);

   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
main_key_down_cb(void *data, int type EINA_UNUSED, void *ev)
{
   Ecore_Event_Key *event = ev;
   Application_Data *ad = data;
   const char *keyname = event->keyname;

   printf("-- %s\n", keyname);

   if (!strcmp("Control_L", keyname))
     ad->ctrl_pressed = EINA_TRUE;
   else if (!strcmp("Shift_L", keyname))
     ad->shift_pressed = EINA_TRUE;
   else if (!strcmp("Alt_L", keyname))
     {
        ad->alt_pressed = EINA_TRUE;

        // show urlbar and multiple bar
        browser_urlbar_show(ad->active_browser);
        browser_multiplebar_show(ad->active_browser);
     }
   else
     {
        if (ad->ctrl_pressed && ad->shift_pressed)
          {
          }
        else if (ad->ctrl_pressed)
          {
             if (*keyname == 'F')
               {
               }
             else if (!strcmp(keyname, "b"))
               {  // Back
                  webview_back(ad->active_browser->active_webview);
               }
             else if (!strcmp(keyname, "f"))
               {  // Forward
                  webview_forward(ad->active_browser->active_webview);
               }
             else if (!strcmp(keyname, "n"))
               {  // Open new window.
                  Browser_Data *bd = browser_add(ad, "about:blank");
                  evas_object_resize(bd->win, ad->default_width, ad->default_height);
                  evas_object_show(bd->win);
                  ad->active_browser = bd;

                  return ECORE_CALLBACK_DONE;
               }
             else if (!strcmp(keyname, "t"))
               {  // Open new tab
                  browser_tab_add(ad->active_browser);
               }
          }
        else if (ad->shift_pressed)
          {
          }
        else if (ad->alt_pressed)
          {
             if (!strcmp(keyname, "d"))
               {
                  browser_urlbar_entry_focus_with_selection(ad->active_browser);
                  return ECORE_CALLBACK_DONE;
               }
          }
        else
          {
             if (*keyname == 'F')
               {
                  switch(keyname[1] - '0')
                    {
                     case 5:
                        webview_reload_bypass_cache(ad->active_browser->active_webview);
                        break;
                    }
               }
          }

     }
   return ECORE_CALLBACK_PASS_ON;
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

        //TODO: add timer to hide menubar/multiplebar
        ad->hide_timer = ecore_timer_add(1, _hide_menu_cb, ad);
     }

   return ECORE_CALLBACK_PASS_ON;
}

EAPI_MAIN int
elm_main(int argc, char** argv)
{
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

   bd = browser_add(&ad, "http://bunhere.tistory.com");
   ad.active_browser = bd;

   evas_object_resize(bd->win, ad.default_width, ad.default_height);
   evas_object_show(bd->win);

   elm_run();

   ewk_shutdown();

   elm_shutdown();

   return 0;
}
ELM_MAIN()
