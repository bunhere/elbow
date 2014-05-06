/*
 * License LGPL-3, see COPYING file at project folder.
 */

#if defined(USE_ELM_WEB)
#include <Elementary.h>
#include "app.h"
#include "browser.h"

static Evas_Object *
_new_window_hook(void *data, Evas_Object *obj, Eina_Bool js EINA_UNUSED, const Elm_Web_Window_Features *wf EINA_UNUSED)
{
   Browser_Data *bd = data;

   fprintf(stderr, "%s\n", __func__);

   //TODO: Check some setting to add new webview as a tab
   Browser_Data *new_bd = browser_add(bd->ad, NULL);
   evas_object_resize(new_bd->win, bd->ad->default_width, bd->ad->default_height);
   evas_object_show(new_bd->win);

   //FIXME: Do not use bd's member here
   return new_bd->active_tab->webview;
}

Evas_Object *webview_ewk_add(Evas_Object *parent, Browser_Data *bd)
{
   Evas_Object *web = elm_web_add(parent);

   elm_web_window_create_hook_set(web, _new_window_hook, bd);
   return web;
}
#endif
