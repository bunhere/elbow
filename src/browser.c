/*
 * Copyright (C) 2013-2014 Ryuan Choi
 */

#include "browser.h"

#include "app.h"
#include "bookmark/bookmark.h"
#include "homescreen.h"
#include "log.h"
#include "webview.h"
#include <Edje.h>
#include <Elementary.h>

#if defined(USE_EWEBKIT)
#include <EWebKit.h>
#elif defined(USE_ELM_WEB)
#if defined(ELM_WEB)
#include <EWebKit.h>
#else
#include <EWebKit2.h>
#endif
#endif

typedef struct {
   Evas_Object *popup;
   Evas_Object *entry;
   Evas_Object *pwd;
   void *data;
} Popup_Data;

static void _urlbar_activated(void *data, Evas_Object *entry, void *event_info);
static void _urlbar_unfocused(void *data, Evas_Object *entry, void *event_info);
static void _urlbar_filter_prepend(void *data, Evas_Object *entry, char **text);

static void _browser_focus_in(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _browser_focus_out(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void _browser_callbacks_register(Browser_Data *bd, Browser_Tab *tab, Evas_Object *webview);
static void _browser_callbacks_deregister(Browser_Data *bd, Browser_Tab *tab, Evas_Object *webview);

static void _progress_update(Browser_Data* bd, float progress);

// TAB
static void _browser_tab_active(Browser_Data *bd, Browser_Tab *active);

static void
_tabbar_item_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Browser_Tab *tab = data;
   Browser_Data *bd = tab->bd;

   _browser_tab_active(bd, tab);
}

static Browser_Tab *
_browser_tab_add(Browser_Data *bd, const char *url)
{
   BROWSER_CALL_LOG("");
   Browser_Tab *new_tab;

   new_tab = malloc(sizeof(Browser_Tab));
   new_tab->bd = bd;
   //new_tab->webview = NULL;
   //new_tab->ewkview = NULL;

   if (url)
     {
        Evas_Object *webview;

        new_tab->homescreen = NULL;

        // webview
        new_tab->webview = webview = webview_add(bd);
        new_tab->ewkview = EWKVIEW(webview);

        _browser_callbacks_register(bd, new_tab, webview);

        webview_url_set(webview, url);
     }
   else
     {
        Evas_Object *homescreen;

        new_tab->webview = NULL;

        // homescreen
        new_tab->homescreen = homescreen = home_screen_add(bd);
     }

   new_tab->toolbar_item = elm_toolbar_item_append(bd->tabbar, NULL, url ? url : "about:home", _tabbar_item_cb, new_tab);
   elm_toolbar_item_selected_set(new_tab->toolbar_item, EINA_TRUE);

   _progress_update(bd, 0);

   bd->tabs = eina_list_append(bd->tabs, new_tab);

   return new_tab;
}

static void
_browser_tab_del(Browser_Data *bd, Browser_Tab *tab, Eina_Bool update_active)
{
   BROWSER_CALL_LOG("");
    if (tab->webview)
        evas_object_del(tab->webview);
    if (tab->homescreen)
        evas_object_del(tab->homescreen);

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

    if (!bd->tabs)
        browser_del(bd);
}

void
_browser_tab_active(Browser_Data *bd, Browser_Tab *active)
{
   BROWSER_CALL_LOG("");

   if (bd->active_tab == active) return;

   Evas_Object *old;
   Evas_Object *tab_content;

   const char *title = NULL;

   if (active->webview)
     {
        tab_content = active->webview;
        elm_object_text_set(bd->urlbar.entry, WEBVIEW_URL(tab_content));
        title = WEBVIEW_TITLE(tab_content);
        elm_win_title_set(bd->win, title);
        _progress_update(bd, 0); //FIXME update as current progress
     }
   else
     {
        tab_content = active->homescreen;
        elm_object_text_set(bd->urlbar.entry, "about:home");
        elm_win_title_set(bd->win, PROJECT_NAME);
        _progress_update(bd, 0);
     }

   bd->active_tab = active;

   old = elm_object_part_content_unset(bd->layout, "content");
   if (old) evas_object_hide(old);

   elm_object_part_content_set(bd->layout, "content", tab_content);
   evas_object_show(tab_content);

   if (bd->active_tab->toolbar_item)
     elm_toolbar_item_selected_set(bd->active_tab->toolbar_item, EINA_TRUE);
}

static void
_browser_tab_url_set(Browser_Tab *tab, Browser_Data *bd, const char *url)
{
   if (!tab->webview)
     {
        Evas_Object *webview;
        tab->webview = webview = webview_add(bd);
        tab->ewkview = EWKVIEW(webview);

        _browser_callbacks_register(bd, tab, webview);

        elm_object_part_content_set(bd->layout, "content", webview);
        evas_object_del(tab->homescreen);
        tab->homescreen = NULL;
     }

   webview_url_set(tab->webview, url);
}

static void
_browser_tab_next(Browser_Tab *tab, Browser_Data *bd)
{
    Eina_List *l, *ltmp = NULL;
    l = eina_list_data_find_list(bd->tabs, tab);

    ltmp = l->next;
    if (!ltmp) ltmp = bd->tabs;

    _browser_tab_active(bd, eina_list_data_get(ltmp));
}

static void
_browser_tab_previous(Browser_Tab *tab, Browser_Data *bd)
{
    Eina_List *l, *ltmp = NULL;
    l = eina_list_data_find_list(bd->tabs, tab);

    ltmp = l->prev;
    if (!ltmp) ltmp = eina_list_last(bd->tabs);

    _browser_tab_active(bd, eina_list_data_get(ltmp));
}

static void
_browser_bookmark_add(Browser_Tab *tab)
{
   if (tab->webview)
     bookmark_add_item(WEBVIEW_URL(tab->webview), WEBVIEW_TITLE(tab->webview));
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
   Browser_Tab *tab = data;
   Browser_Data *bd = tab->bd;
   if (!bd->active_tab || bd->active_tab != tab) return;

   elm_object_disabled_set(bd->urlbar.back_button, !webview_back_possible(o)); 
   elm_object_disabled_set(bd->urlbar.forward_button, !webview_forward_possible(o));
}

#if defined(USE_EWEBKIT2) || (defined(USE_ELM_WEB) && defined(ELM_WEB2))
static void
_auth_cancel(void *data, Evas_Object *obj, void *event_info)
{
    Popup_Data *popup_data = (Popup_Data *)data;
    Ewk_Auth_Request *request = (Ewk_Auth_Request *)popup_data->data;

    ewk_auth_request_cancel(request);
    ewk_object_unref(request);

    evas_object_del(popup_data->popup);
    free(popup_data);
}

static void
_auth_ok(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
    Popup_Data *popup_data = (Popup_Data *)data;
    Ewk_Auth_Request *request = (Ewk_Auth_Request *)popup_data->data;

    const char *username = elm_entry_entry_get(popup_data->entry);
    const char *password = elm_entry_entry_get(popup_data->pwd);
    ewk_auth_request_authenticate(request, username, password);
    ewk_object_unref(request);

    evas_object_del(popup_data->popup);
    free(popup_data);
}

static void
_auth_entry_activated_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _auth_ok(data, NULL, NULL);
}

static void
_authentication_request_cb(void *data, Evas_Object *o, void *event_info)
{
   BROWSER_CALL_LOG("");

   Evas_Object *popup, *entry, *pwd;

   Browser_Tab *tab = data;
   Browser_Data *bd = tab->bd;

   Popup_Data *popup_data = (Popup_Data *)malloc(sizeof(Popup_Data));
   if (!popup_data) return;

   Ewk_Auth_Request *request = ewk_object_ref((Ewk_Auth_Request *)event_info);
   popup_data->data = request;

   popup_data->popup = popup = elm_popup_add(bd->win);
   evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_part_text_set(popup, "title,text", "Authentication Required");

   /* Popup Content */
   Evas_Object *vbox = elm_box_add(popup);
   elm_box_padding_set(vbox, 0, 4);
   evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(vbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(popup, vbox);
   evas_object_show(vbox);

   /* Authentication message */
   Evas_Object *label = elm_label_add(popup);
   elm_label_line_wrap_set(label, ELM_WRAP_WORD);
   Eina_Strbuf *auth_text = eina_strbuf_new();
   const char *host = ewk_auth_request_host_get(request);
   const char *realm = ewk_auth_request_realm_get(request);
   eina_strbuf_append_printf(auth_text, "A username and password are being requested by %s. The site says: \"%s\"", host, realm ? realm : "");
   elm_object_text_set(label, eina_strbuf_string_get(auth_text));
   eina_strbuf_free(auth_text);
   evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(vbox, label);
   evas_object_show(label);

   /* Credential table */
   Evas_Object *table = elm_table_add(popup);
   elm_table_padding_set(table, 2, 2);
   elm_table_homogeneous_set(table, EINA_TRUE);
   evas_object_size_hint_weight_set(table, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(table, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(vbox, table);
   evas_object_show(table);

   /* Username row */
   Evas_Object *username_label = elm_label_add(popup);
   elm_object_text_set(username_label, "Username:");
   evas_object_size_hint_weight_set(username_label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(username_label, 1, EVAS_HINT_FILL);
   elm_table_pack(table, username_label, 0, 0, 1, 1);
   evas_object_show(username_label);

   popup_data->entry = entry = elm_entry_add(popup);
   elm_entry_scrollable_set(entry, EINA_TRUE);
   elm_entry_single_line_set(entry, EINA_TRUE);
   evas_object_smart_callback_add(entry, "activated", _auth_ok, popup_data);
   evas_object_smart_callback_add(entry, "aborted", _auth_cancel, popup_data);
   const char *suggested_username = ewk_auth_request_suggested_username_get(request);
   elm_entry_entry_set(entry, suggested_username ? suggested_username : "");
   evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(table, entry, 1, 0, 2, 1);
   elm_object_focus_set(entry, EINA_TRUE);
   evas_object_show(entry);

   /* Password row */
   Evas_Object *password_label = elm_label_add(popup);
   elm_object_text_set(password_label, "Password:");
   evas_object_size_hint_weight_set(password_label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(password_label, 1, EVAS_HINT_FILL);
   elm_table_pack(table, password_label, 0, 1, 1, 1);
   evas_object_show(password_label);

   popup_data->pwd = pwd = elm_entry_add(popup);
   elm_entry_scrollable_set(pwd, EINA_TRUE);
   elm_entry_single_line_set(pwd, EINA_TRUE);
   elm_entry_password_set(pwd, EINA_TRUE);
   evas_object_smart_callback_add(pwd, "activated", _auth_ok, popup_data);
   evas_object_smart_callback_add(pwd, "aborted", _auth_cancel, popup_data);
   evas_object_size_hint_weight_set(pwd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(pwd, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(table, pwd, 1, 1, 2, 1);
   evas_object_show(pwd);

   /* Popup buttons */
   Evas_Object *cancel_button = elm_button_add(popup);
   elm_object_text_set(cancel_button, "Cancel");
   elm_object_part_content_set(popup, "button1", cancel_button);
   evas_object_smart_callback_add(cancel_button, "clicked", _auth_cancel, popup_data);
   Evas_Object *ok_button = elm_button_add(popup);
   elm_object_text_set(ok_button, "OK");
   elm_object_part_content_set(popup, "button2", ok_button);
   evas_object_smart_callback_add(ok_button, "clicked", _auth_ok, popup_data);
   evas_object_show(popup);
}
#endif

static void
_inspector_create_cb(void *data, Evas_Object *o, void *event_info)
{
   BROWSER_CALL_LOG("");
   //TODO: needToImplement
}

static void
_inspector_close_cb(void *data, Evas_Object *o, void *event_info)
{
   BROWSER_CALL_LOG("");
   //TODO: needToImplement
}

static void
_title_changed_cb(void *data, Evas_Object *o, void *event_info)
{
   Browser_Tab *tab = data;
   Browser_Data *bd = tab->bd;

   if (!event_info) return;
#if defined(USE_EWEBKIT) || defined(ELM_WEB)
    const char* title = ((const Ewk_Text_With_Direction*)event_info)->string;
#else
    const char* title = (const char *)event_info;
#endif

   if (tab->toolbar_item)
     {
         //TODO: I want to change label only.
         Elm_Toolbar_Item_State *current = elm_toolbar_item_state_get(tab->toolbar_item);
         Elm_Toolbar_Item_State *new = elm_toolbar_item_state_add(tab->toolbar_item, NULL, title, _tabbar_item_cb, tab);
         elm_toolbar_item_state_set(tab->toolbar_item, new);
         elm_toolbar_item_state_del(tab->toolbar_item, current);
     }

   if (!bd->active_tab || bd->active_tab != tab) return;
   elm_win_title_set(bd->win, title);
}

static void
_url_changed_cb(void *data, Evas_Object *o, void *event_info)
{
   BROWSER_CALL_LOG("");

   Browser_Tab *tab = data;
   Browser_Data *bd = tab->bd;
   if (!bd->active_tab || bd->active_tab != tab) return;

   elm_object_text_set(bd->urlbar.entry, event_info);
   bd->user_focused = EINA_FALSE;
}

static void
_load_error_cb(void *data, Evas_Object *o, void *event_info)
{
   BROWSER_CALL_LOG("");
   //TODO: needToImplement
}

typedef struct {
    Edje_Message_Float_Set set;
    double val;
} _Progress;

static void
_progress_update(Browser_Data* bd, float progress)
{
   _Progress progress_msg;
   Edje_Message_Float_Set *msg = (Edje_Message_Float_Set*)&progress_msg;
   msg->count = 1;
   msg->val[0] = progress;

   edje_object_message_send(elm_layout_edje_get(bd->layout), EDJE_MESSAGE_FLOAT_SET, 0, msg);
}

static void
_load_progress_cb(void *data, Evas_Object *o, void *event_info)
{
   Browser_Tab *tab = data;
   Browser_Data *bd = tab->bd;
   if (!bd->active_tab || bd->active_tab != tab) return;

   _progress_update(bd, *((double *)event_info));
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
   Browser_Tab *tab = data;
   Browser_Data *bd = tab->bd;
   if (bd->active_tab != tab) return;

   if (!bd->user_focused)
     {
        elm_object_focus_set(bd->urlbar.entry, EINA_FALSE);
        webview_focus_set(bd->active_tab->webview, EINA_TRUE);
     }

   _progress_update(bd, 0);

#if !defined(USE_EWEBKIT2)
   _back_forward_list_changed_cb(tab, o, event_info);
#endif

#if defined(USE_EWEBKIT2) || (defined(USE_ELM_WEB) && defined(ELM_WEB2))
   ewk_view_script_execute(o, "var _wkrss = document.querySelector(\"link[type='application/rss+xml']\"); _wkrss ? _wkrss.href : \"\";", script_execute_result_cb, bd);
#endif
}

#if defined(USE_EWEBKIT2) || (defined(USE_ELM_WEB) && defined(ELM_WEB2))
static void
_favicon_changed_cb(Ewk_Favicon_Database *database, const char *url, void *user_data)
{
   BROWSER_CALL_LOG("");
   Browser_Data *bd = user_data;
   Evas_Object* favicon;
   favicon = ewk_favicon_database_icon_get(database, url, evas_object_evas_get(bd->layout));
   if (favicon)
     {
        //FIXME: just for test
        evas_object_move(favicon, 0, 0);
        evas_object_resize(favicon, 16, 16);
        evas_object_show(favicon);
     }
}
#endif

static void
_browser_callbacks_register(Browser_Data *bd, Browser_Tab *tab, Evas_Object *webview)
{
#define SMART_CALLBACK_ADD(signal, func) \
       evas_object_smart_callback_add(EWKVIEW(webview), signal, func, tab)

#if defined(USE_EWEBKIT2) || (defined(USE_ELM_WEB) && defined(ELM_WEB2))
   SMART_CALLBACK_ADD("authentication,request", _authentication_request_cb);
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

#if defined(USE_EWEBKIT2) || (defined(USE_ELM_WEB) && defined(ELM_WEB2))
   ewk_favicon_database_icon_change_callback_add(ewk_context_favicon_database_get(ewk_view_context_get(EWKVIEW(webview))), _favicon_changed_cb, bd);
#endif

   evas_object_event_callback_add(webview, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, bd);
}

static void
_browser_callbacks_deregister(Browser_Data *bd, Browser_Tab *tab, Evas_Object *webview)
{
#define SMART_CALLBACK_DEL(signal, func) \
       evas_object_smart_callback_del_full(EWKVIEW(webview), signal, func, tab)

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

#undef SMART_CALLBACK_ADD

#if defined(USE_EWEBKIT2) || (defined(USE_ELM_WEB) && defined(ELM_WEB2))
   ewk_favicon_database_icon_change_callback_del(ewk_context_favicon_database_get(ewk_view_context_get(EWKVIEW(webview))), _favicon_changed_cb);
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
browser_add(Application_Data *ad, const char *url)
{
   Browser_Data *bd;
   Evas_Object *en;

   bd = malloc(sizeof(Browser_Data));
   if (!bd) return NULL;

   bd->tabs = NULL;
   bd->active_tab = NULL;
   bd->destroyed = EINA_FALSE;

   bd->ad = ad;
   ad->browsers = eina_list_append(ad->browsers, bd);

   // window
   bd->win = elm_win_util_standard_add(elm_app_name_get(), PROJECT_NAME);
   elm_win_focus_highlight_enabled_set(bd->win, EINA_TRUE);
   evas_object_smart_callback_add(bd->win, "delete,request", win_delete_request_cb, bd);

   evas_object_event_callback_add(bd->win, EVAS_CALLBACK_FOCUS_IN, _browser_focus_in, bd);
   evas_object_event_callback_add(bd->win, EVAS_CALLBACK_FOCUS_OUT, _browser_focus_out, bd);

   // layout
   bd->layout = elm_layout_add(bd->win);
   elm_layout_file_set(bd->layout, ad->main_layout_path, "main_layout");
   elm_win_resize_object_add(bd->win, bd->layout);
   evas_object_show(bd->layout);

   // tabbar
   bd->tabbar = elm_toolbar_add(bd->win);
   elm_toolbar_align_set(bd->tabbar, 0.0);
   elm_toolbar_shrink_mode_set(bd->tabbar, ELM_TOOLBAR_SHRINK_SCROLL);
   evas_object_size_hint_align_set(bd->tabbar, EVAS_HINT_FILL, 0.0);
   evas_object_show(bd->tabbar);

   elm_object_part_content_set(bd->layout, "tabbar", bd->tabbar);

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
   elm_entry_input_panel_layout_set(bd->urlbar.entry, ELM_INPUT_PANEL_LAYOUT_URL);
   evas_object_size_hint_weight_set(bd->urlbar.entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bd->urlbar.entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(bd->urlbar.entry, "activated", _urlbar_activated, bd);
   evas_object_smart_callback_add(bd->urlbar.entry, "unfocused", _urlbar_unfocused, bd);
   elm_box_pack_end(bd->urlbar.bar, bd->urlbar.entry);
   evas_object_show(bd->urlbar.entry);

   elm_object_part_content_set(bd->layout, "urlbar", bd->urlbar.bar);

   // default tab
   Browser_Tab *new_tab;
   new_tab = _browser_tab_add(bd, url);

   _browser_tab_active(bd, new_tab);

   // multi tab bar
   bd->multiplebar.activated = EINA_FALSE;

   //elm_object_text_set(bd->urlbar.entry, "about:blank");

   return bd;
}

void
browser_del(Browser_Data *bd)
{
   if (bd->destroyed) return;
   bd->destroyed = EINA_TRUE;

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
        _browser_tab_url_set(bd->active_tab, bd, url);
     }
   else
     {
        Eina_Strbuf *buf = eina_strbuf_new();
        eina_strbuf_append_printf(buf, "http://%s", url);

        char *url_with_scheme = eina_strbuf_string_steal(buf);
        _browser_tab_url_set(bd->active_tab, bd, url_with_scheme);
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

static void
_browser_focus_in(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   BROWSER_CALL_LOG("%p", obj);
   Browser_Data *bd = data;
   if (bd->ad->active_browser == bd) return;
   bd->ad->active_browser = bd;
}

static void
_browser_focus_out(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   BROWSER_CALL_LOG("%p", obj);
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
          if (!strcmp(keyname, "Tab"))
            {
               _browser_tab_previous(bd->active_tab, bd);
            }
       }
     else if (ctrl)
       {
          if (*keyname == 'F')
            {
            }
          else if (!strcmp(keyname, "d"))
            {
               _browser_bookmark_add(bd->active_tab);
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
               Browser_Data *new_bd = browser_add(ad, NULL);
               evas_object_resize(new_bd->win, ad->default_width, ad->default_height);
               evas_object_show(new_bd->win);

               // Change new active browser;
               ad->active_browser = new_bd;

               return ECORE_CALLBACK_DONE;
            }
          else if (!strcmp(keyname, "t"))
            {  // Open new tab
               Browser_Tab *new_tab;
               new_tab = _browser_tab_add(bd, NULL);
               _browser_tab_active(bd, new_tab);

               return ECORE_CALLBACK_DONE;
            }
          else if (!strcmp(keyname, "w"))
            {  // Open new tab
               _browser_tab_del(bd, bd->active_tab, EINA_TRUE);
            }
          else if (!strcmp(keyname, "Tab"))
            {
               _browser_tab_next(bd->active_tab, bd);
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
