#pragma once
#include <gui/canvas.h>
#include "utils/list.h"
#include "graphics/buffer.h"
#include "f0ge_types.h"
#include "node.h"
#include "component.h"

void init_engine(EngineConfig config);
void set_scene(Node *root);
void start_loop();
void cleanup_engine();

void node_added(Node *node);
void set_renderer_dirty();

// calls component->end and removes the tree from the renderer
void node_removed(Node *node);
// same as node_removed, but also releases the lists for the components and node tree
void node_free(Node *node);

void change_ui_renderer(void (*render_ui)(void *gameState, Canvas *canvas));