/*
 * Copyright (C) 2014 Ryuan Choi
 */

#include "bookmark.h"

#include "log.h"
#include <sqlite3.h>
#include <Eina.h>

struct _Bookmark_Item
{
   Eina_Stringshare *url;
   Eina_Stringshare *title;
};
typedef struct _Bookmark_Item Bookmark_Item;

struct _Bookmark
{
   Eina_List *items;
};
typedef struct _Bookmark Bookmark;

static Bookmark *_bookmark_instance = NULL;

static Bookmark *
_bookmark_create()
{
   Bookmark *ret;

   ret = (Bookmark *)malloc(sizeof(Bookmark));
   ret->items = NULL;

   return ret;
}

static Bookmark *
_bookmark_get()
{
   if (!_bookmark_instance) _bookmark_instance = _bookmark_create();

   return _bookmark_instance;
}

Eina_Bool
bookmark_add_item(Eina_Stringshare *title, Eina_Stringshare *url)
{
   BROWSER_CALL_LOG("");

   Bookmark *bm = _bookmark_get();
   void *data;
   Eina_List *l;

   // check exist
   EINA_LIST_FOREACH(bm->items, l, data) {
      Bookmark_Item *item = data;
      if (!strcmp(url, item->url)) return EINA_FALSE;
   }

   Bookmark_Item *added = (Bookmark_Item*)malloc(sizeof(Bookmark_Item));
   added->url = eina_stringshare_ref(url);
   added->title = eina_stringshare_ref(title);

   BROWSER_LOGD("%s/%s", added->url, added->title);

   return EINA_FALSE;
}

Eina_List *
bookmark_items()
{
   return _bookmark_get()->items;
}

void
bookmark_shutdown()
{
   if (!_bookmark_instance) return;

   free(_bookmark_instance);
}
