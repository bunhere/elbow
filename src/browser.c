/*
 * Copyright (C) 2013-2014 Ryuan Choi
 */

#include <Elementary.h>
#include "app.h"
#include "browser.h"
#include "log.h"
#include "webview.h"

#if defined(USE_EWEBKIT)
#include <EWebKit.h>
#elif defined(USE_ELM_WEB)
#if defined(ELM_WEB)
#include <EWebKit.h>
#else
#include <EWebKit2.h>
#endif
#endif

static void _urlbar_activated(void *data, Evas_Object *entry, void *event_info);
static void _urlbar_unfocused(void *data, Evas_Object *entry, void *event_info);
static void _urlbar_filter_prepend(void *data, Evas_Object *entry, char **text);

static void _browser_callbacks_register(Browser_Data *bd, Evas_Object *webview);
static void _browser_callbacks_deregister(Browser_Data *bd, Evas_Object *webview);

// TAB
static void _browser_tab_active(Browser_Data *bd, Browser_Tab *active);

static Browser_Tab *
_browser_tab_add(Browser_Data *bd)
{
   BROWSER_CALL_LOG("");
   Browser_Tab *new_tab;

   new_tab = malloc(sizeof(Browser_Tab));
   //new_tab->webview = NULL;
   //new_tab->ewkview = NULL;

   // webview
   Evas_Object *webview;
   new_tab->webview = webview = webview_add(bd);
   new_tab->ewkview = EWKVIEW(webview);

   _browser_callbacks_register(bd, webview);

   bd->tabs = eina_list_append(bd->tabs, new_tab);

   return new_tab;
}

static void
_browser_tab_del(Browser_Data *bd, Browser_Tab *tab, Eina_Bool update_active)
{
   BROWSER_CALL_LOG("");
    if (tab->webview)
        evas_object_del(tab->webview);

    Eina_List *l, *ltmp = NULL;
    l = eina_list_data_find_list(bd->tabs, tab);

    if (update_active)
      {
         if (l->next) ltmp = l->next;
         else if (l->prev) ltmp = l->prev;

         if (ltmp)
           _browser_tab_active(bd, eina_list_data_get(ltmp));
      }

    bd->tabs = eina_list_remove_list(bd->tabs, l);

    free(tab);
}

void
_browser_tab_active(Browser_Data *bd, Browser_Tab *active)
{
   BROWSER_CALL_LOG("");

   if (bd->active_tab == active) return;

   Evas_Object *old;

   bd->active_tab = active;

   old = elm_object_part_content_get(bd->layout, "content");
   if (old) evas_object_hide(old);

   elm_object_part_content_set(bd->layout, "content", active->webview);
   evas_object_show(active->webview);
}

static Eina_Bool
_hide_menu_cb(void *data)
{
   BROWSER_CALL_LOG("");

   Application_Data *ad = data;
   browser_urlbar_hide(ad->active_browser);
   browser_multiplebar_hide(ad->active_browser);

   return ECORE_CALLBACK_CANCEL;
}

static void
win_delete_request_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Browser_Data *bd = data;

   browser_del(bd);
}

static void
_back_forward_list_changed_cb(void *data, Evas_Object *o, void *event_info)
{
   Browser_Data *bd = data;
   if (bd->active_tab->ewkview != o) return;

   elm_object_disabled_set(bd->urlbar.back_button, !webview_back_possible(o)); 
   elm_object_disabled_set(bd->urlbar.forward_button, !webview_forward_possible(o));
}

static void
_inspector_create_cb(void *userData, Evas_Object *o, void *eventInfo)
{
   BROWSER_CALL_LOG("");
   //TODO: needToImplement
}

static void
_inspector_close_cb(void *userData, Evas_Object *o, void *eventInfo)
{
   BROWSER_CALL_LOG("");
   //TODO: needToImplement
}

static void
_title_changed_cb(void *data, Evas_Object *o, void *event_info)
{
   Browser_Data *bd = data;
   if (bd->active_tab->ewkview != o) return;

   if (!event_info) return;

#if defined(USE_EWEBKIT) || defined(ELM_WEB)
    const char* title = ((const Ewk_Text_With_Direction*)event_info)->string;
#else
    const char* title = (const char *)event_info;
#endif

   elm_win_title_set(bd->win, title);
}

static void
_url_changed_cb(void *data, Evas_Object *o, void *event_info)
{
   BROWSER_CALL_LOG("");

   Browser_Data *bd = data;
   if (bd->active_tab->ewkview != o) return;

   elm_object_text_set(bd->urlbar.entry, event_info);
   bd->user_focused = EINA_FALSE;
}

static void
_load_error_cb(void *data, Evas_Object *o, void *event_info)
{
   BROWSER_CALL_LOG("");
   //TODO: needToImplement
}

static void
_load_progress_cb(void *data, Evas_Object *o, void *event_info)
{
   BROWSER_CALL_LOG("");

   Browser_Data *bd = data;
   if (bd->active_tab->ewkview != o) return;

   //double *progress = (double *)event_info;
   //TODO: needToImplement
}

static void
_mouse_down_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Browser_Data *bd = data;
   Evas_Event_Mouse_Down *ev = event_info;

   if (ev->button != 1)
     browser_urlbar_show(bd);
   else
     {
        //TODO: add timer to hide menubar/multiplebar
        bd->ad->hide_timer = ecore_timer_add(0.3, _hide_menu_cb, bd->ad);
     }
}

static void
script_execute_result_cb(Evas_Object *o, const char *value, void *data)
{
   BROWSER_CALL_LOG("");

   Browser_Data *bd = data;
   if (bd->active_tab->ewkview != o) return;

   BROWSER_LOGD("[%s]", value);
}

static void
_load_finished_cb(void *data, Evas_Object *o, void *event_info)
{
   Browser_Data *bd = data;
   if (bd->active_tab->ewkview != o) return;

   if (!bd->user_focused)
     {
        elm_object_focus_set(bd->urlbar.entry, EINA_FALSE);
        webview_focus_set(bd->active_tab->webview, EINA_TRUE);
     }

#if !defined(USE_EWEBKIT2)
   _back_forward_list_changed_cb(data, o, event_info);
#endif

#if defined(USE_EWEBKIT2) || defined(ELM_WEB2)
   ewk_view_script_execute(o, "var _wkrss = document.querySelector(\"link[type='application/rss+xml']\"); _wkrss ? _wkrss.href : \"\";", script_execute_result_cb, bd);
#endif
}

static void
_favicon_changed_cb(void *data, Evas_Object *o, void *event_info)
{
#if defined(USE_EWEBKIT2) || defined(ELM_WEB2)
   Evas_Object* favicon;
   favicon = ewk_favicon_database_icon_get(ewk_context_favicon_database_get(ewk_view_context_get(o)), ewk_view_url_get(o), evas_object_evas_get(o));
   if (favicon)
     {
        evas_object_move(favicon, 0, 0);
        evas_object_resize(favicon, 100, 100);
        evas_object_show(favicon);
     }
#endif
}

static void
_browser_callbacks_register(Browser_Data *bd, Evas_Object *webview)
{
#define SMART_CALLBACK_ADD(signal, func) \
       evas_object_smart_callback_add(EWKVIEW(webview), signal, func, bd)

#if defined(USE_WEBKIT2)
   SMART_CALLBACK_ADD("back,forward,list,changed", _back_forward_list_changed_cb);
#endif
   SMART_CALLBACK_ADD("inspector,view,create", _inspector_create_cb);
   SMART_CALLBACK_ADD("inspector,view,close", _inspector_close_cb);
   SMART_CALLBACK_ADD("title,changed", _title_changed_cb);
#if defined(USE_EWEBKIT)
   SMART_CALLBACK_ADD("uri,changed", _url_changed_cb);
#else
   SMART_CALLBACK_ADD("url,changed", _url_changed_cb);
#endif
   SMART_CALLBACK_ADD("load,error", _load_error_cb);
   SMART_CALLBACK_ADD("load,progress", _load_progress_cb);
   SMART_CALLBACK_ADD("load,finished", _load_finished_cb);
   SMART_CALLBACK_ADD("favicon,changed", _favicon_changed_cb);

#undef SMART_CALLBACK_ADD

   evas_object_event_callback_add(webview, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, bd);
}

static void
_browser_callbacks_deregister(Browser_Data *bd, Evas_Object *webview)
{
#define SMART_CALLBACK_DEL(signal, func) \
       evas_object_smart_callback_del_full(EWKVIEW(webview), signal, func, bd)

#if defined(USE_WEBKIT2)
   SMART_CALLBACK_DEL("back,forward,list,changed", _back_forward_list_changed_cb);
#endif
   SMART_CALLBACK_DEL("inspector,view,create", _inspector_create_cb);
   SMART_CALLBACK_DEL("inspector,view,close", _inspector_close_cb);
   SMART_CALLBACK_DEL("title,changed", _title_changed_cb);
#if defined(USE_EWEBKIT)
   SMART_CALLBACK_DEL("uri,changed", _url_changed_cb);
#else
   SMART_CALLBACK_DEL("url,changed", _url_changed_cb);
#endif
   SMART_CALLBACK_DEL("load,error", _load_error_cb);
   SMART_CALLBACK_DEL("load,progress", _load_progress_cb);
   SMART_CALLBACK_DEL("load,finished", _load_finished_cb);
   SMART_CALLBACK_DEL("favicon,changed", _favicon_changed_cb);

#undef SMART_CALLBACK_ADD
}

static Evas_Object *
_urlbar_button_add(Evas_Object *win, const char * const icon_name)
{
   Evas_Object *btn, *icon;

   btn = elm_button_add(win);
   icon = elm_icon_add(win);

   elm_icon_standard_set(icon, icon_name);
   elm_object_part_content_set(btn, "icon", icon);

   return btn;
}

Browser_Data *
browser_add(Application_Data *ad, const char *url)
{
   Browser_Data *bd;
   Evas_Object *en;

   bd = malloc(sizeof(Browser_Data));
   if (!bd) return NULL;

   bd->tabs = NULL;

   bd->ad = ad;
   ad->browsers = eina_list_append(ad->browsers, bd);

   // window
   bd->win = elm_win_util_standard_add(elm_app_name_get(), PROJECT_NAME);
   elm_win_focus_highlight_enabled_set(bd->win, EINA_TRUE);
   evas_object_smart_callback_add(bd->win, "delete,request", win_delete_request_cb, bd);

   // layout
   bd->layout = elm_layout_add(bd->win);
   elm_layout_file_set(bd->layout, ad->main_layout_path, "main_layout");
   elm_win_resize_object_add(bd->win, bd->layout);
   evas_object_show(bd->layout);

   // urlbar
   bd->urlbar.activated = EINA_FALSE;
   bd->urlbar.bar = elm_box_add(bd->win);
   elm_box_horizontal_set(bd->urlbar.bar, EINA_TRUE);
   evas_object_size_hint_weight_set(bd->urlbar.bar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bd->urlbar.bar, EVAS_HINT_FILL, 0.0);
   evas_object_show(bd->urlbar.bar);

   // urlbar.back_button
   bd->urlbar.back_button = _urlbar_button_add(bd->win, "arrow_left");
   elm_object_disabled_set(bd->urlbar.back_button, EINA_TRUE);
   evas_object_size_hint_weight_set(bd->urlbar.back_button, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bd->urlbar.back_button, 0.0, 0.5);
   elm_box_pack_end(bd->urlbar.bar, bd->urlbar.back_button);
   evas_object_show(bd->urlbar.back_button);

   // urlbar.forward_button
   bd->urlbar.forward_button = _urlbar_button_add(bd->win, "arrow_right");
   elm_object_disabled_set(bd->urlbar.forward_button, EINA_TRUE);
   evas_object_size_hint_weight_set(bd->urlbar.forward_button, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bd->urlbar.forward_button, 0.0, 0.5);
   elm_box_pack_end(bd->urlbar.bar, bd->urlbar.forward_button);
   evas_object_show(bd->urlbar.forward_button);

   // urlbar.entry
   bd->urlbar.entry = elm_entry_add(bd->win);
   elm_entry_single_line_set(bd->urlbar.entry, EINA_TRUE);
   evas_object_size_hint_weight_set(bd->urlbar.entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bd->urlbar.entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(bd->urlbar.entry, "activated", _urlbar_activated, bd);
   evas_object_smart_callback_add(bd->urlbar.entry, "unfocused", _urlbar_unfocused, bd);
   elm_box_pack_end(bd->urlbar.bar, bd->urlbar.entry);
   evas_object_show(bd->urlbar.entry);

   elm_object_part_content_set(bd->layout, "urlbar", bd->urlbar.bar);

   // default tab
   Browser_Tab *new_tab;
   new_tab = _browser_tab_add(bd);

   _browser_tab_active(bd, new_tab);

   // multi tab bar
   bd->multiplebar.activated = EINA_FALSE;

   //elm_object_text_set(bd->urlbar.entry, "about:blank");

   if (url)
     webview_url_set(new_tab->webview, url);

   return bd;
}

void
browser_del(Browser_Data *bd)
{
   application_remove_browser(bd->ad, bd);
   void *it;

   EINA_LIST_FREE(bd->tabs, it)
     {
         _browser_tab_del(bd, it, EINA_FALSE);
     }

   evas_object_del(bd->win);
   free(bd);
}

void
browser_multiplebar_show(Browser_Data *bd)
{
   if (bd->multiplebar.activated) return;
   bd->multiplebar.activated = EINA_TRUE;

   elm_object_signal_emit(bd->layout, "multiplebar,on", "");
}

void
browser_multiplebar_hide(Browser_Data *bd)
{
   if (!bd->multiplebar.activated) return;
   bd->multiplebar.activated = EINA_FALSE;

   //TODO: needToImplement
   elm_object_signal_emit(bd->layout, "multiplebar,off", "");
}

// urlbar
static void
_urlbar_activated(void *data, Evas_Object *entry, void *event_info)
{
   Browser_Data *bd = data;

   const char *markup_url = elm_entry_entry_get(entry);
   char *url = elm_entry_markup_to_utf8(markup_url);

   if (strstr(url, "://") || !strcasecmp(url, "about:blank"))
     {
        // user may be written some scheme.
        webview_url_set(bd->active_tab->webview, url);
     }
   else
     {
        Eina_Strbuf *buf = eina_strbuf_new();
        eina_strbuf_append_printf(buf, "http://%s", url);

        char *url_with_scheme = eina_strbuf_string_steal(buf);
        webview_url_set(bd->active_tab->webview, url_with_scheme);
        eina_strbuf_free(buf); 
     }

   elm_object_focus_set(bd->urlbar.entry, EINA_FALSE);
   webview_focus_set(bd->active_tab->webview, EINA_TRUE);
   bd->user_focused = EINA_FALSE;

   free(url);
}

static void
_urlbar_unfocused(void *data, Evas_Object *entry, void *event_info)
{
   Browser_Data *bd = data;
   elm_entry_select_none(bd->urlbar.entry);
}

void
browser_urlbar_show(Browser_Data *bd)
{
   if (bd->urlbar.activated) return;
   bd->urlbar.activated = EINA_TRUE;

   elm_object_signal_emit(bd->layout, "urlbar,on", "");
}

void
browser_urlbar_hide(Browser_Data *bd)
{
   if (!bd->urlbar.activated) return;
   if (bd->ad->alt_pressed) return;

   if (elm_object_focus_get(bd->urlbar.entry))
     {
        return;
     }

   bd->urlbar.activated = EINA_FALSE;

   elm_object_signal_emit(bd->layout, "urlbar,off", "");
}

void
browser_urlbar_entry_focus_with_selection(Browser_Data *bd)
{
   elm_object_focus_set(bd->urlbar.entry, EINA_TRUE);
   elm_entry_select_all(bd->urlbar.entry);

   bd->user_focused = EINA_TRUE;
}

Eina_Bool
browser_keydown(Browser_Data *bd, const char *keyname, Eina_Bool ctrl, Eina_Bool alt, Eina_Bool shift)
{
   BROWSER_CALL_LOG("");

   BROWSER_LOGD("-- (%p)%s\n", bd, keyname);

     if (ctrl && shift)
       {
       }
     else if (ctrl)
       {
          if (*keyname == 'F')
            {
            }
          else if (!strcmp(keyname, "b"))
            {  // Back
               webview_back(bd->active_tab->webview);
            }
          else if (!strcmp(keyname, "f"))
            {  // Forward
               webview_forward(bd->active_tab->webview);
            }
          else if (!strcmp(keyname, "n"))
            {  // Open new window.
               Application_Data *ad = bd->ad;
               Browser_Data *new_bd = browser_add(ad, "about:blank");
               evas_object_resize(new_bd->win, ad->default_width, ad->default_height);
               evas_object_show(new_bd->win);

               // Change new active browser;
               ad->active_browser = new_bd;

               return ECORE_CALLBACK_DONE;
            }
          else if (!strcmp(keyname, "t"))
            {  // Open new tab
               Browser_Tab *new_tab;
               new_tab = _browser_tab_add(bd);
               _browser_tab_active(bd, new_tab);

               webview_url_set(new_tab->webview, application_default_url(bd->ad));
            }
          else if (!strcmp(keyname, "w"))
            {  // Open new tab
               _browser_tab_del(bd, bd->active_tab, EINA_TRUE);
            }
       }
     else if (shift)
       {
       }
     else if (alt)
       {
          if (!strcmp(keyname, "d"))
            {
               browser_urlbar_entry_focus_with_selection(bd);
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
                     webview_reload_bypass_cache(bd->active_tab->webview);
                     break;
                 }
            }
       }

   return ECORE_CALLBACK_PASS_ON;
}
