#pragma once

#include <furi.h>
#include "../f0ge.h"

typedef struct Tweener Tweener;

struct Tweener {
    void *data;
    float length;
    float delay;
    float t;
    bool finished;
    bool (*update)(Tweener *);
    bool (*end)(Tweener *);
};

void tweener_start(Tweener *tweener);
void tweener_end(Tweener *tweener);

void tweener_update();

void tweener_prepare(RuntimeData *data);

void tweener_cleanup();
