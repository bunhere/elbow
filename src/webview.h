/*
 * Copyright (C) 2013 Ryuan Choi
 *
 * License LGPL-3, see COPYING file at project folder.
 */
#ifndef webview_h
#define webview_h

#include "browser.h"

Evas_Object *webview_add(Browser_Data *bd);

/*
 * Each backend have similar APIs but different name.
 */
#if defined(USE_EWEBKIT)
#define webview_back                  ewk_view_back
#define webview_back_possible         ewk_view_back_possible
#define webview_focus_set             evas_object_focus_set
#define webview_forward               ewk_view_forward
#define webview_forward_possible      ewk_view_forward_possible
#define webview_reload_bypass_cache   ewk_view_reload_full
#define webview_url_set               ewk_view_uri_set
#define webview_user_agent_set        ewk_view_setting_user_agent_set

Evas_Object *webview_ewk_add(Evas_Object* parent, Browser_Data* bd);

#elif defined(USE_EWEBKIT2)
#define webview_back                  ewk_view_back
#define webview_back_possible         ewk_view_back_possible
#define webview_focus_set             evas_object_focus_set
#define webview_forward               ewk_view_forward
#define webview_forward_possible      ewk_view_forward_possible
#define webview_reload_bypass_cache   ewk_view_reload_bypass_cache
#define webview_url_set               ewk_view_url_set
#define webview_user_agent_set        ewk_view_user_agent_set

Evas_Object *webview_ewk2_add(Evas_Object* parent, Browser_Data* bd);

#else
#define webview_back                  elm_web_back
#define webview_back_possible         elm_web_back_possible_get
#define webview_focus_set             elm_object_focus_set
#define webview_forward               elm_web_forward
#define webview_forward_possible      elm_web_forward_possible_get
#define webview_reload_bypass_cache   elm_web_reload_full
#define webview_url_set               elm_web_url_set
#define webview_user_agent_set        elm_web_useragent_set

#endif

#endif // webview_h
