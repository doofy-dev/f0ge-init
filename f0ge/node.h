#pragma once
#include <furi.h>
#include "math/matrix.h"
#include "utils/list.h"
#include "graphics/buffer.h"
typedef struct RenderData RenderData;
typedef struct RenderingData RenderingData;
typedef struct Node Node;

#define MAKE_NODE() (Node){ \
    .active=false, \
    .transform=MAKE_TRANSFORM(), \
    .parent=NULL, \
    .children=NULL, \
    .components=NULL, \
    .sprite=NULL, \
    .render_callback=NULL \
}

struct Node {
    bool active;
    Transform transform;
    Node *parent;
    List *children;
    List *components;
    RenderData *sprite;

    RenderingData *_rendering_data; //reference to engine cache

    void (*render_callback)(Node *self, Buffer *buffer);
};
