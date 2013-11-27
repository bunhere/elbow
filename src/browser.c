/*
 * Copyright (C) 2013 Ryuan Choi
 *
 * License LGPL-3, see COPYING file at project folder.
 */

#include <Elementary.h>
#include "app.h"
#include "browser.h"
#include "webview.h"

#if defined(USE_EWEBKIT) || defined(USE_ELM_WEB)
#include <EWebKit.h>
#endif

static void _urlbar_activated(void *data, Evas_Object *entry, void *event_info);
static void _urlbar_filter_prepend(void *data, Evas_Object *entry, char **text);

static void
win_delete_request_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Browser_Data *bd = data;

   application_remove_browser(bd->ad, bd);

   evas_object_del(bd->win);
   free(bd);
}

static void
_back_forward_list_changed_cb(void *data, Evas_Object *o, void *event_info)
{
   Browser_Data *bd = data;
   elm_object_disabled_set(bd->urlbar.back_button, !webview_back_possible(o)); 
   elm_object_disabled_set(bd->urlbar.forward_button, !webview_forward_possible(o));
}

static void
_inspector_create_cb(void *userData, Evas_Object *o, void *eventInfo)
{
   printf("%s\n", __func__);
   //TODO: needToImplement
}

static void
_inspector_close_cb(void *userData, Evas_Object *o, void *eventInfo)
{
   printf("%s\n", __func__);
   //TODO: needToImplement
}

static void
_title_changed_cb(void *data, Evas_Object *o, void *event_info)
{
   Browser_Data *bd = data;

   if (!event_info) return;

#if defined(USE_EWEBKIT) || defined(USE_ELM_WEB)
    const char* title = ((const Ewk_Text_With_Direction*)event_info)->string;
#else
    const char* title = (const char *)event_info;
#endif

   elm_win_title_set(bd->win, title);
}

static void
_url_changed_cb(void *data, Evas_Object *o, void *event_info)
{
   Browser_Data *bd = data;

   printf("%s\n", __func__);
   elm_object_text_set(bd->urlbar.entry, event_info);
}

static void
_load_error_cb(void *data, Evas_Object *o, void *event_info)
{
   printf("%s\n", __func__);
   //TODO: needToImplement
}

static void
_load_progress_cb(void *data, Evas_Object *o, void *event_info)
{
   double *progress = (double *)event_info;
   printf("%s %f\n", __func__, *progress);
   //TODO: needToImplement
}

static void
_load_finished_cb(void *data, Evas_Object *o, void *event_info)
{
   Browser_Data *bd = data;
   elm_object_focus_set(bd->urlbar.entry, EINA_FALSE);
   webview_focus_set(bd->active_webview, EINA_TRUE);
   browser_urlbar_hide(bd);

#if !defined(USE_EWEBKIT2)
   _back_forward_list_changed_cb(data, o, event_info);
#endif
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
browser_add(Application_Data *ad)
{
   Browser_Data *bd;
   Evas_Object *en;

   bd = malloc(sizeof(Browser_Data));
   if (!bd) return NULL;

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
   evas_object_size_hint_weight_set(bd->urlbar.bar, EVAS_HINT_EXPAND, 0.0);
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
   elm_box_pack_end(bd->urlbar.bar, bd->urlbar.entry);
   evas_object_show(bd->urlbar.entry);

   elm_object_part_content_set(bd->layout, "urlbar", bd->urlbar.bar);

   // multi tab bar
   bd->multiplebar.activated = EINA_FALSE;

   // webview
   bd->active_webview = webview_add(bd);
   evas_object_show(bd->active_webview);
   elm_object_part_content_set(bd->layout, "content", bd->active_webview);
   bd->webviews = eina_list_append(NULL, bd->active_webview);

#define SMART_CALLBACK_ADD(signal, func) \
       evas_object_smart_callback_add(bd->active_webview, signal, func, bd)

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

#undef SMART_CALLBACK_ADD

   //elm_object_text_set(bd->urlbar.entry, "about:blank");

   //FIXME: Just for test
   webview_url_set(bd->active_webview, "http://bunhere.tistory.com");

   return bd;
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
        webview_url_set(bd->active_webview, url);
     }
   else
     {
        Eina_Strbuf *buf = eina_strbuf_new();
        eina_strbuf_append_printf(buf, "http://%s", url);

        char *url_with_scheme = eina_strbuf_string_steal(buf);
        webview_url_set(bd->active_webview, url_with_scheme);
        eina_strbuf_free(buf); 
     }

   free(url);
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
}

void
browser_tab_add(Browser_Data *bd)
{
   //TODO: needToImplement
}
