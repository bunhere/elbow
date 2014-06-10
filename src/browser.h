/*
 * Copyright (C) 2013 Ryuan Choi
 */
#ifndef browser_h
#define browser_h

#include <Evas.h>
#include <Elementary.h>

typedef struct _Application_Data Application_Data;

typedef struct _Browser_Data Browser_Data;

struct _Browser_Tab
{
   Browser_Data *bd; /* browser */
   Evas_Object *webview;
   Evas_Object *ewkview; /* Just for easy access */
   Evas_Object *homescreen;

   Elm_Object_Item *toolbar_item;
};
typedef struct _Browser_Tab Browser_Tab;

struct _Browser_Data
{
   Evas_Object *win;
   Evas_Object *layout;

   Application_Data *ad;

   Browser_Tab *active_tab;
   Eina_List *tabs;

   Evas_Object *tabbar;
   struct {
      Evas_Object *bar;
      Evas_Object *back_button;
      Evas_Object *forward_button;
      Evas_Object *entry;
      Evas_Object *refresh_button;
      Evas_Object *home_button;
      Eina_Bool activated;
   } urlbar;

   struct {
      Evas_Object *bar;
      Eina_Bool activated;
   } multiplebar;

   Eina_Bool user_focused;
   Eina_Bool destroyed;
};

Browser_Data *browser_add(Application_Data *ad, const char *url);
void browser_del(Browser_Data *bd);
void browser_multiplebar_show(Browser_Data *bd);
void browser_urlbar_entry_focus_with_selection(Browser_Data *bd);
void browser_urlbar_hide(Browser_Data *bd);
void browser_urlbar_show(Browser_Data *bd);

void browser_multiplebar_hide(Browser_Data *bd);

Eina_Bool browser_keydown(Browser_Data *bd, const char *keyname, Eina_Bool ctrl, Eina_Bool alt, Eina_Bool shift);
//void browser_keyup(Browser_Data *bd, void *ev);

#endif // browser_h
