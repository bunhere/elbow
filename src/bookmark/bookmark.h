/*
 * Copyright (C) 2014 Ryuan Choi
 */

#ifndef bookmark_h
#define bookmark_h

#include <Eina.h>

Eina_Bool bookmark_add_item(Eina_Stringshare *title, Eina_Stringshare *url);

Eina_List *bookmark_items();

void bookmark_shutdown();

#endif
