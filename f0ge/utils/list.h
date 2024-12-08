#pragma once

#include <furi.h>
#define FOREACH(var, list) for (ListItem* var = list->head; var != NULL; var = var->next)

typedef struct List List;
typedef struct ListItem ListItem;

//TODO maybe add a release callback to make the data release configurable
struct ListItem {
    void *data;
    struct ListItem *next;
    struct ListItem *prev;
};

struct List{
    ListItem *head;
    ListItem *tail;
    size_t count;
};

List *list_make();
List *list_from(size_t count, ...);
//frees everything
void list_free(List *list);
//clears the list, but not data
void list_clear(List *list);

//clears the list, and data
void list_free_data(List *list);

void list_push_back(void *data, List *list);

void list_push_front(void *data, List *list);

void *list_pop_back(List *list);

void *list_pop_front(List *list);

void *list_pop_at(size_t index, List *list);

void list_remove_item(void *data, List *list);

void list_remove_at(size_t index, List *list);

List *list_splice(size_t index, size_t count, List *list);

void *list_peek_front(List *list);
void *list_get_index(List *list, size_t index);
void *list_peek_index(List *list, size_t index);

void *list_peek_back(List *list);