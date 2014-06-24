#include <Eina.h>
#include <stdio.h>
#include "bookmark.h"

int main()
{
    eina_init();

    Eina_Stringshare *title, *url;

    title = eina_stringshare_add("title1");
    url = eina_stringshare_add("url1");

    printf("1. bookmark_item_add\n");
    bookmark_item_add(url, title);

    printf("2. bookmark_items\n");
    Eina_List *list = bookmark_items();

    Eina_List *l;
    void *item;
    EINA_LIST_FOREACH(list, l, item)
      {
         printf("%s:%s\n", 
                 bookmark_item_url_get(item),
                 bookmark_item_title_get(item));
      }

    bookmark_shutdown();
    eina_shutdown();
}
