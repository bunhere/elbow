/*
 * Copyright (C) 2013 Ryuan Choi
 */

#include <Elementary.h>
#include "browser.h"
#include "webview.h"

#if defined(USE_EWEBKIT2)
#include <EWebKit2.h>
#endif

//Just for test
#define MOBILE_USER_AGENT \
  "Mozilla/5.0 (iPhone; U; CPU like Mac OS X; en)  AppleWebKit/420+ (KHTML, like Gecko) Version/3.0 Mobile/1A543a Safari/419.3 /" 

Evas_Object *
webview_add(Browser_Data *bd)
{
   Evas_Object *web;

   web = webview_ewk_add(bd->win, bd);

   //FIXME : Check installation path(for release) with build directory(for development)
   ewk_view_theme_set(EWKVIEW(web), THEME_BUILD_PATH "/webkit.edj");

   ewk_context_favicon_database_directory_set(ewk_view_context_get(EWKVIEW(web)), NULL);

   ewk_settings_plugins_enabled_set(ewk_page_group_settings_get(ewk_view_page_group_get(EWKVIEW(web))), EINA_FALSE);

   //webview_user_agent_set(web, MOBILE_USER_AGENT);

   return web;
}
