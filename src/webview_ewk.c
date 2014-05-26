/*
 * Copyright (C) 2013 Ryuan Choi
 *
 * License LGPL-3, see COPYING file at project folder.
 */

#if defined(USE_EWEBKIT)
#include <EWebKit.h>
#include "browser.h"

static Ewk_View_Smart_Class _parent_sc = EWK_VIEW_SMART_CLASS_INIT_NULL;

typedef struct _view_data view_data;
struct _view_data
{
   Ewk_View_Smart_Data base;
   Browser_Data *bd;
};

static void _ewk_smart_add(Evas_Object *o)
{
   view_data *sd = (view_data *)calloc(1, sizeof(view_data));
   evas_object_smart_data_set(o, sd);

   _parent_sc.sc.add(o);
}

void _ewk_smart_del(Evas_Object *o)
{
   _parent_sc.sc.del(o);
}

static Evas_Object *_ewk_window_create(Ewk_View_Smart_Data *sd, Eina_Bool javascript, const Ewk_Window_Features *windowFeatures)
{
   int x = 0;
   int y = 0;
   int width = 0;
   int height = 0;

   ewk_window_features_int_property_get(windowFeatures, &x, &y, &width, &height);

   /*
   BrowserConfig config;
   if (width)
      config.width = width;
   if (height)
      config.height = height;

   Browser* browser = Browser::create(config);
   browser->show();

   BrowserContent* content = browser->contentsAt(0);
   return content->object();
   */
   return NULL;
}

static Eina_Bool _ewk_key_down(Ewk_View_Smart_Data *sd, const Evas_Event_Key_Down *down)
{
   return _parent_sc.key_down(sd, down);

}

static Eina_Bool
_mouse_down_cb(Ewk_View_Smart_Data *esd, const Evas_Event_Mouse_Down *ev)
{
   view_data *sd = (view_data *)esd;

   elm_object_focus_set(sd->bd->win, EINA_FALSE);
   evas_object_focus_set(esd->self, EINA_TRUE);

   _parent_sc.mouse_down(esd, ev);
}

Evas_Object *webview_ewk_add(Evas_Object *parent, Browser_Data *bd)
{
   static Evas_Smart *smart = 0;
   Evas *canvas = evas_object_evas_get(parent);

   if (!smart)
     {
        static Ewk_View_Smart_Class api = EWK_VIEW_SMART_CLASS_INIT_NAME_VERSION("EWK_View_Demo");
        ewk_view_smart_set(&api);
        ewk_view_smart_set(&_parent_sc);
        
        api.sc.add = _ewk_smart_add;
        api.sc.del = _ewk_smart_del;
        
        api.window_create = _ewk_window_create;
        api.key_down = _ewk_key_down;
        
        api.mouse_down = _mouse_down_cb;

        smart = evas_smart_class_new(&api.sc);
        if (!smart) return 0;
     }

   Evas_Object *ewkView = evas_object_smart_add(canvas, smart);
   view_data *sd = (view_data *)evas_object_smart_data_get(ewkView);
   sd->bd= bd;

   return ewkView;
}
#endif
