/*
 * Copyright (C) 2014 Ryuan Choi
 */

#include "bookmark.h"
#include <Eina.h>

struct _Bookmark_Item
{
   Eina_Stringshare *title;
   Eina_Stringshare *url;
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

   // Just for demo

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
   Bookmark_Item *item = (Bookmark_Item *)malloc(sizeof(Bookmark_Item));

   return EINA_FALSE;
}

Eina_List *
bookmark_items()
{
   return _bookmark_get()->items;
}

void
bookmark_shudown()
{
   if (!_bookmark_instance) return;

   free(_bookmark_instance);
}
