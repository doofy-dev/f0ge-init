#pragma once
#include <furi.h>
#include "math/matrix.h"
#include "utils/list.h"
#include "graphics/buffer.h"
typedef struct RenderData RenderData;
typedef struct Node Node;

#define MAKE_NODE() (Node){ \
    .active=false, \
    .transform=MAKE_TRANSFORM(), \
    .parent=NULL, \
    .children=NULL, \
    .components=NULL, \
    .sprite=NULL, \
    .render=NULL \
}

struct Node {
    bool active;
    Transform transform;
    Node *parent;
    List *children;
    List *components;
    RenderData *sprite;

    void (*render)(Node *self, Buffer *buffer);
};
