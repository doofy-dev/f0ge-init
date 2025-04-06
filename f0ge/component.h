#pragma once
#include <furi.h>
typedef struct Component Component;
typedef struct Node Node;

#define MAKE_COMPONENT() (Component){ \
    .start=NULL, \
    .update=NULL, \
    .end=NULL, \
    .data=NULL \
}

struct Component{
    void (*start)(Node *self, void *data);
    void (*update)(Node *self, float delta, void *data);
    void (*end)(Node *self, void *data);
    void *data;
};