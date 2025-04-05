#pragma once
#include <furi.h>
#include <gui/gui.h>
#include <notification/notification.h>
#include "graphics/buffer.h"
#include "utils/list.h"
typedef struct Node Node;
typedef struct EngineConfig EngineConfig;

typedef struct Buffer Buffer;
typedef struct {
    Canvas *canvas;
    Gui *gui;
    Buffer *buffer;
} RenderInstance;

struct EngineConfig{
    bool muted;
    bool backlight;
    uint8_t physics_fps;
    uint8_t render_fps;
    uint8_t volume;
    void *gameState;
    void (*render_ui)(void *gameState, Canvas *canvas);
};

typedef struct {
    bool dirty;
    bool exit;
    bool muted;
    uint8_t volume;
    float delta_time;
    FuriPubSub *input;
    InputType inputState[6];
    FuriPubSubSubscription *input_subscription;
    FuriMutex *update_mutex;
    RenderInstance renderInstance;
    NotificationApp *notification_app;
    List* renderers;
    Node *root;
} RuntimeData;