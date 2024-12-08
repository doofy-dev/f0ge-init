#pragma once
#include <furi.h>
#include "math/matrix.h"
#include "utils/list.h"
#include "graphics/buffer.h"

typedef struct Node Node;

struct Node {
    bool active;
    Transform transform;
    Node *parent;
    List* children;
    List* components;
    void (*render)(Node *self, Buffer *buffer);
};
