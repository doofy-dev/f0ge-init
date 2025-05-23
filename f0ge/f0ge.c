#include <notification/notification_messages.h>
#include "f0ge.h"
#include "component.h"
#include "node.h"
#include "utils/helpers.h"
#include "utils/tweener.h"
#include "utils/scheduler.h"
#include "utils/audio.h"
#include "graphics/asset.h"
#include "graphics/render.h"

//TODO LEAKING MEMORY somewhere
static RuntimeData runtimeData;
static EngineConfig engineConfig;

struct RenderingData {
    Node *node;
    Vector cachedCorners[4];
};

RenderingData *make_rendering_data(Node *node) {
    RenderingData *data = allocate(sizeof(RenderingData));
    data->node = node;
    for (int i = 0; i < 4; i++) {
        data->cachedCorners[i] = node->sprite->poly.corners[i];
    }
    node->_rendering_data = data;
    return data;
}

InputType get_key_state(InputKey key) {
    return runtimeData.inputState[key];
}

bool is_down(InputKey key) {
    return runtimeData.inputState[key] == InputTypeLong || runtimeData.inputState[key] == InputTypePress;
}

bool is_pressed(InputKey key) {
    return runtimeData.inputState[key] == InputTypePress;
}

bool is_up(InputKey key) {
    return runtimeData.inputState[key] == InputTypeRelease;
}

static void gui_input_events_callback(const void *value, void *ctx) {
    RuntimeData *data = ctx;
    const InputEvent *event = value;

    if (event->type == InputTypeRepeat) return;

    furi_mutex_acquire(data->update_mutex, FuriWaitForever);
    if (event->key == InputKeyBack && event->type == InputTypeLong) {
        data->exit = true;
    }


    data->inputState[event->key] = event->type;

    furi_mutex_release(data->update_mutex);
}

void change_ui_renderer(void (*render_ui)(void *gameState, Canvas *canvas)) {
    engineConfig.render_ui = render_ui;
}

void init_engine(EngineConfig config) {
    engineConfig = config;
    runtimeData.update_mutex = (FuriMutex *) furi_mutex_alloc(FuriMutexTypeNormal);
    runtimeData.exit = false;
    runtimeData.volume = config.volume;
    runtimeData.muted = config.muted;
    runtimeData.input = furi_record_open(RECORD_INPUT_EVENTS);
    runtimeData.renderInstance.gui = furi_record_open(RECORD_GUI);
    runtimeData.renderInstance.canvas = gui_direct_draw_acquire(runtimeData.renderInstance.gui);
    runtimeData.input_subscription = furi_pubsub_subscribe(runtimeData.input, gui_input_events_callback, &runtimeData);
    runtimeData.notification_app = (NotificationApp *) furi_record_open(RECORD_NOTIFICATION);

    if (config.backlight)
        notification_message_block(runtimeData.notification_app, &sequence_display_backlight_enforce_on);

    runtimeData.renderInstance.buffer = buffer_create(SCREEN_WIDTH, SCREEN_HEIGHT, false);

    runtimeData.renderers = list_make();

    tweener_prepare(&runtimeData);
    scheduler_prepare(&runtimeData);
    setup_audio(runtimeData.notification_app);
    for (uint8_t i = 0; i < 6; i++) {
        runtimeData.inputState[i] = InputTypeMAX;
    }
}

void node_added(Node *node) {
    if (!node) return;
    set_renderer_dirty();
    node->transform.dirty = true;
    // matrix_reset(&(node->transform.transformation_matrix));
    node->active = true;

    if (node->children == NULL) node->children = list_make();
    if (node->components == NULL) node->components = list_make();

    if (node->render_callback || node->sprite) {
        list_push_back(
            make_rendering_data(node),
            runtimeData.renderers
        );
    }

    FOREACH(n, node->components) {
        Component *c = n->data;
        /*c->started = true;
        c->node = node;*/
        if (c->start) c->start(node, c->data);
    }

    FOREACH(n, node->children) {
        Node *c = n->data;
        c->parent = node;
        node_added(c);
    }
}

void node_removed(Node *node) {
    if (!node) return;
    node->_rendering_data = NULL;
    set_renderer_dirty();
    FOREACH(n, node->children) {
        node_removed(n->data);
    }
    FOREACH(n, node->components) {
        Component *c = n->data;
        if (c->end) c->end(node, c->data);
    }

    //remove renderer if had one
    if (node->sprite) {
        FOREACH(n, runtimeData.renderers) {
            if (n->data == node) {
                list_remove_item(n, runtimeData.renderers);
                release(n);
                break;
            }
        }
    }
}

void node_free(Node *node) {
    if (!node) return;
    FOREACH(n, node->children) {
        node_free(n->data);
    }
    node->_rendering_data = NULL;

    FOREACH(n, node->components) {
        Component *c = n->data;
        if (c->end) c->end(node, c->data);
    }

    //remove renderer if had one
    if (node->sprite) {
        FOREACH(n, runtimeData.renderers) {
            if (n->data == node) {
                list_remove_item(n, runtimeData.renderers);
                release(n);
                break;
            }
        }
    }

    list_clear(node->components);
    list_clear(node->children);

    release(node);
}

void cleanup_engine() {
    node_free(runtimeData.root);
    tweener_cleanup();
    scheduler_cleanup();
    list_free_data(runtimeData.renderers);
    release(runtimeData.renderers);

    asset_cleanup();

    buffer_release(runtimeData.renderInstance.buffer);

    furi_mutex_free(runtimeData.update_mutex);

    furi_pubsub_unsubscribe(runtimeData.input, runtimeData.input_subscription);
    gui_direct_draw_release(runtimeData.renderInstance.gui);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_INPUT_EVENTS);

    runtimeData.renderInstance.canvas = NULL;

    notification_message_block(runtimeData.notification_app, &sequence_display_backlight_enforce_auto);
    furi_record_close(RECORD_NOTIFICATION);
}

Matrix temp_matrix;

void update_transform(Node *node) {
    if (node->transform.dirty == true) {
        set_renderer_dirty();
        //compute from top to bottom to have a correct matrix for relative positions
        compute_transformation_matrix(&(node->transform), node->parent ? &(node->parent->transform) : NULL);


        if (node->sprite) {
            for (int i = 0; i < 4; i++) {
                matrix_mul_vector(&(node->transform.transformation_matrix), &(node->sprite->poly.corners[i]),
                                  &(node->_rendering_data->cachedCorners[i]));
            }
        }
    }


    check_pointer(node->children);
    FOREACH(child, node->children) {
        check_pointer(child);
        check_pointer(child->data);
        Node *c = child->data;
        check_pointer(c);
        c->transform.dirty = node->transform.dirty || c->transform.dirty;
        update_transform(child->data);
    }
    node->transform.dirty = false;
}


void set_scene(Node *root) {
    if (runtimeData.root)
        node_removed(runtimeData.root);
    runtimeData.root = root;
    node_added(root);
    update_transform(root);
}

void add_component(Node *node, Component *component) {
    if (node->components == NULL) node->components = list_make();
    list_push_back(component, node->components);
    if (!node->active) return;
    component->start(node, component->data);
}

void add_child(Node *parent, Node *child) {
    if (parent->children == NULL) parent->children = list_make();

    list_push_back(child, parent->children);
    if (!parent->active) return;
    child->parent = parent;
    node_added(child);
    update_transform(child);
}

void update(Node *node) {
    if (!node->active) return;

    FOREACH(comp, node->components) {
        Component *c = comp->data;
        if (c->update) c->update(node, runtimeData.delta_time, c->data);
        if (node->transform.dirty) update_transform(node);
    }

    FOREACH(child, node->children) {
        update(child->data);
    }
}

void render() {
    FOREACH(r, runtimeData.renderers) {
        RenderingData *rd = r->data;

        set_transform(&(rd->node->transform.transformation_matrix));
        if (rd->node->render_callback) {
            // FURI_LOG_D("-","---------------------------");
            // Timer *t = timer_start("node render");
            rd->node->render_callback(rd->node, runtimeData.renderInstance.buffer);
            // timer_end(t);
        } else if (rd->node->sprite) {
            rasterize(runtimeData.renderInstance.buffer, rd->node->sprite, rd->cachedCorners);
        }
    }
}

void set_renderer_dirty() {
    runtimeData.dirty = true;
}

void start_loop() {
    furi_thread_set_current_priority(FuriThreadPriorityIdle);
    size_t curr_frame_time = furi_get_tick();
    //Failsafe, if 0, set it to a reasonable fps
    if (engineConfig.render_fps == 0) engineConfig.render_fps = 20;

    const uint32_t FPS = 1000 / engineConfig.render_fps;
    size_t last_frame_time = curr_frame_time - FPS;
    size_t delta = 0;

    while (!runtimeData.exit) {
        if (furi_mutex_acquire(runtimeData.update_mutex, 25) != FuriStatusOk)
            continue;

        curr_frame_time = furi_get_tick();
        delta = curr_frame_time - last_frame_time;
        // Timer *t = timer_start("update");

        //Skip processing if delta is below the set update rate
        if (delta >= FPS) {
            runtimeData.delta_time = (float) (delta) / 1000.f/* / 64000000.0f*/;
            last_frame_time = curr_frame_time;

            update(runtimeData.root);

            scheduler_update();
            tweener_update();
            update_audio();

            buffer_clear(runtimeData.renderInstance.buffer);
            // t = timer_start("render");
            if (runtimeData.dirty) {
                render();
            }
        }

        // timer_end(t);
        // FURI_LOG_D("-","---------------------------");

        // Draw the back buffer and UI
        // t = timer_start("xbm");
        buffer_render(runtimeData.renderInstance.buffer, runtimeData.renderInstance.canvas);
        // timer_end(t);

        if (engineConfig.render_ui) {
            engineConfig.render_ui(engineConfig.gameState, runtimeData.renderInstance.canvas);
        }

        canvas_commit(runtimeData.renderInstance.canvas);


        furi_mutex_release(runtimeData.update_mutex);
        furi_thread_yield();
    }

    cleanup_engine();
}
