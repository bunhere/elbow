/*
 * Copyright (C) 2013 Ryuan Choi
 *
 * License LGPL-3, see COPYING file at project folder.
 */

#include <Elementary.h>
#include "browser.h"
#include "webview.h"

//Just for test
#define MOBILE_USER_AGENT \
  "Mozilla/5.0 (iPhone; U; CPU like Mac OS X; en)  AppleWebKit/420+ (KHTML, like Gecko) Version/3.0 Mobile/1A543a Safari/419.3 /" 

Evas_Object *
webview_add(Browser_Data *bd)
{
   Evas_Object *web;

#if defined(USE_EWEBKIT)
   web = webview_ewk_add(bd->win, bd);
#elif defined(USE_EWEBKIT2)
   web = webview_ewk2_add(bd->win, bd);
#else
   web = elm_web_add(bd->win);
#endif

   //FIXME : Check installation path(for release) with build directory(for development)
   ewk_view_theme_set(webview_webkit_view_get(web), THEME_BUILD_PATH "/webkit.edj");

   webview_user_agent_set(web, MOBILE_USER_AGENT);

   return web;
}
