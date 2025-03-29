#pragma once
#include <furi.h>
typedef struct Component Component;
typedef struct Node Node;

struct Component{
    void (*start)(Node *self);
    void (*update)(Node *self, float delta);
    void (*end)(Node *self);
};