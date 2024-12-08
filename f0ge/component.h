#pragma once
#include <furi.h>
typedef struct Component Component;
typedef struct Node Node;

struct Component{
    Node *node;
    bool started;
    void (*start)(Component *self);
    void (*update)(Component *self, float delta);
    void (*end)(Component *self);
};