/*
 * Copyright (C) 2014 Ryuan Choi
 */

#include "bookmark.h"

#include "log.h"
#include <sqlite3.h>
#include <sys/stat.h>
#include <Eina.h>

struct _Bookmark_Item
{
   Eina_Stringshare *url;
   Eina_Stringshare *title;
   Eina_Bool modified;
};

struct _Bookmark
{
   Eina_List *items;
};
typedef struct _Bookmark Bookmark;

static Bookmark *_bookmark_instance = NULL;

static const char *
_data_dir()
{
   static const char *data_dir = NULL;

   if (data_dir) return data_dir;

   static char *home_directory = NULL;
   if (!home_directory) {
       home_directory = getenv("HOME");
       if (!home_directory)
           home_directory = "/tmp";
   }
       
   data_dir = eina_stringshare_printf("%s/.elbow", home_directory);

   return data_dir;
}

static const char *
_bookmark_path()
{
   static const char *bookmark_dir = NULL;
   if (bookmark_dir) return bookmark_dir;

   bookmark_dir = eina_stringshare_printf("%s/bookmark.sqlite3", _data_dir());
   return bookmark_dir;
}

static Eina_Bool
_validate_data_directory()
{
   struct stat file_info;
   if (stat(_data_dir(), &file_info)) {
       printf("mkdir\n");
       mkdir(_data_dir(), S_IRWXU);
   }

   if (stat(_data_dir(), &file_info)) {
       fprintf(stderr, "Error %s\n", _data_dir());
       return EINA_FALSE;
   }

   return EINA_TRUE;
}

static Bookmark_Item *
_bookmark_item_add_internal(Eina_Stringshare *url, Eina_Stringshare *title)
{
   Bookmark_Item *added = (Bookmark_Item*)malloc(sizeof(Bookmark_Item));
   added->url = eina_stringshare_ref(url);
   added->title = eina_stringshare_ref(title);

   return added;
}

static Bookmark *
_bookmark_create()
{
   BROWSER_CALL_LOG("");
   Bookmark *bm;

   if (!_validate_data_directory())
     return NULL;

   bm = (Bookmark *)malloc(sizeof(Bookmark));
   bm->items = NULL;

   sqlite3* pDb;
   char *errmsg;
   int ret;
   ret = sqlite3_open_v2(_bookmark_path(), &pDb, SQLITE_OPEN_READONLY, NULL);
   if (ret)
     {
        return bm;
     }

   sqlite3_stmt *stmt;
   if (sqlite3_prepare_v2(pDb,
                       "SELECT url, title FROM _bookmark_item",
                       -1,
                       &stmt,
                       NULL) != SQLITE_OK)
     goto close;

   while (EINA_TRUE)
     {
        ret = sqlite3_step(stmt);
        if (ret == SQLITE_ROW)
          {
             const char *url = sqlite3_column_text(stmt, 0);
             const char *title = sqlite3_column_text(stmt, 1);
             Bookmark_Item *item = _bookmark_item_add_internal(eina_stringshare_add(url), eina_stringshare_add(title));
             bm->items = eina_list_append(bm->items, item);
          }
        else
          break;
     }
          
close:
   sqlite3_close(pDb);
   return bm;
}

static Bookmark *
_bookmark_get()
{
   BROWSER_CALL_LOG("");

   if (!_bookmark_instance) _bookmark_instance = _bookmark_create();

   return _bookmark_instance;
}

Eina_Bool
bookmark_item_add(Eina_Stringshare *url, Eina_Stringshare *title)
{
   BROWSER_CALL_LOG("");

   Bookmark *bm = _bookmark_get();
   if (!bm) return EINA_FALSE;

   void *data;
   Eina_List *l;

   // check exist
   EINA_LIST_FOREACH(bm->items, l, data) {
      Bookmark_Item *item = data;
      if (!strcmp(url, item->url)) return EINA_FALSE;
   }

   Bookmark_Item *added = _bookmark_item_add_internal(title, url);
   bm->items = eina_list_append(bm->items, added);

   BROWSER_LOGD("%s/%s", added->url, added->title);

   sqlite3* pDb;
   char *errmsg;
   int ret;
   sqlite3_open(_bookmark_path(), &pDb);
   ret = sqlite3_exec(pDb,
                      "PRAGMA synchronous=OFF; PRAGMA count_changes=OFF; PRAGMA temp_store=memory;",
                      NULL,
                      NULL,
                      &errmsg);

   if (sqlite3_exec(pDb,
                    "CREATE TABLE IF NOT EXISTS _bookmark_item(url,title);",
                    NULL, NULL, &errmsg) != SQLITE_OK)
     {
        fprintf(stderr, "%s:%d Error\n", __func__, __LINE__);
        return EINA_FALSE;
     }

   sqlite3_stmt *stmt;
   if (sqlite3_prepare(pDb,
                       "INSERT INTO _bookmark_item VALUES(?,?)",
                       -1,
                       &stmt,
                       0) != SQLITE_OK)
     {
        fprintf(stderr, "%s:%d Error\n", __func__, __LINE__);
        return EINA_FALSE;
     }

   sqlite3_bind_text(stmt, 1, url, eina_stringshare_strlen(url), SQLITE_STATIC);
   sqlite3_bind_text(stmt, 2, title, eina_stringshare_strlen(title), SQLITE_STATIC);

   if (sqlite3_step(stmt) != SQLITE_DONE)
     {
        fprintf(stderr, "%s:%d Error\n", __func__, __LINE__);
        return EINA_FALSE;
     }
   sqlite3_reset(stmt);

   sqlite3_close(pDb);
   return EINA_TRUE;
}

Eina_List *
bookmark_items()
{
   return _bookmark_get()->items;
}

Eina_Stringshare *
bookmark_item_title_get(const Bookmark_Item *item)
{
   return item->title;
}

Eina_Stringshare *
bookmark_item_url_get(const Bookmark_Item *item)
{
   return item->url;
}

void
bookmark_shutdown()
{
   if (!_bookmark_instance) return;

   free(_bookmark_instance);
}
