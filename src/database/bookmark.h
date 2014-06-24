/*
 * Copyright (C) 2014 Ryuan Choi
 */

#ifndef bookmark_h
#define bookmark_h

#include <Eina.h>

typedef struct _Bookmark_Item Bookmark_Item;

Eina_Bool bookmark_item_add(Eina_Stringshare *url, Eina_Stringshare *title);

Eina_List *bookmark_items();

Eina_Stringshare *bookmark_item_title_get(const Bookmark_Item *item);
Eina_Stringshare *bookmark_item_url_get(const Bookmark_Item *item);

void bookmark_shutdown();

#endif
