#include <notification/notification_messages.h>
#include "f0ge.h"
#include "component.h"
#include "node.h"
#include "utils/helpers.h"
#include "utils/tweener.h"
#include "utils/scheduler.h"
#include "utils/audio.h"
#include "graphics/asset.h"

//TODO LEAKING MEMORY somewhere
static RuntimeData runtimeData;
static EngineConfig engineConfig;

static void gui_input_events_callback(const void *value, void *ctx) {
    RuntimeData *data = ctx;
    furi_mutex_acquire(data->update_mutex, FuriWaitForever);
    const InputEvent *event = value;
    FURI_LOG_D("INPUT", "GET");
    if (event->type == InputTypeLong)
        FURI_LOG_D("INPUT", "LONG");
    if (event->key == InputKeyBack)
        FURI_LOG_D("INPUT", "BACK");

    if (event->key == InputKeyBack && event->type == InputTypeLong) {
        FURI_LOG_D("INPUT", "exiting");
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
}

void node_added(Node *node) {
    if (!node) return;
    set_renderer_dirty();
    node->transform.dirty = true;
    // matrix_reset(&(node->transform.transformation_matrix));
    node->active = true;

    if (node->children == NULL) node->children = list_make();
    if (node->components == NULL) node->components = list_make();

    if (node->render) {
        list_push_back(node, runtimeData.renderers);
    }

    FOREACH(n, node->components) {
        Component *c = n->data;
        c->started = true;
        c->node = node;
        if (c->start) c->start(c);
    }

    FOREACH(n, node->children) {
        Node *c = n->data;
        c->parent = node;
        node_added(c);
    }
}

void node_removed(Node *node) {
    if (!node) return;
    set_renderer_dirty();
    FOREACH(n, node->children) {
        node_removed(n->data);
    }
    FOREACH(n, node->components) {
        Component *c = n->data;
        if (c->end) c->end(c);
    }
    list_remove_item(node, runtimeData.renderers);
}

void node_free(Node *node) {
    if (!node) return;
    FOREACH(n, node->children) {
        node_free(n->data);
    }

    FOREACH(n, node->components) {
        Component *c = n->data;
        if (c->end) c->end(c);
    }

    list_remove_item(node, runtimeData.renderers);
    list_clear(node->components);
    list_clear(node->children);

    release(node);
}

void cleanup_engine() {
    node_free(runtimeData.root);
    tweener_cleanup();
    scheduler_cleanup();
    list_clear(runtimeData.renderers);
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
    if (true || node->transform.dirty == true) {
        set_renderer_dirty();
        //compute from top to bottom to have a correct matrix for relative positions
        compute_transformation_matrix(&(node->transform), node->parent ? &(node->parent->transform) : NULL);

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


void update(Node *node) {
    if (!node->active) return;

    FOREACH(comp, node->components) {
        Component *c = comp->data;
        if (!c->started) {
            c->node = node;
            if (c->start) c->start(c);
            c->started = true;
        }

        if (c->update) c->update(c, runtimeData.delta_time);
    }

    FOREACH(child, node->children) {
        update(child->data);
    }
}

void render() {
    FOREACH(r, runtimeData.renderers) {
        Node *node = r->data;
        if (node->render) {
            buffer_set_transform(&(node->transform.transformation_matrix));
            node->render(node, runtimeData.renderInstance.buffer);
        }
    }
}

void set_renderer_dirty() {
    runtimeData.dirty = true;
}

void start_loop() {
    uint8_t i = 0;
    furi_thread_set_current_priority(FuriThreadPriorityIdle);
    size_t curr_frame_time = curr_time();
    size_t last_frame_time = curr_frame_time;

    while (!runtimeData.exit) {
        if (furi_mutex_acquire(runtimeData.update_mutex, 25) != FuriStatusOk)
            continue;
        curr_frame_time = curr_time();

        runtimeData.delta_time = (float) (curr_frame_time - last_frame_time) / 64000000.0f;
        last_frame_time = curr_frame_time;
        update(runtimeData.root);
        update_transform(runtimeData.root);

        scheduler_update();
        tweener_update();
        update_audio();

        buffer_clear(runtimeData.renderInstance.buffer);
        if (runtimeData.dirty) {
            render();
        }

        buffer_render(runtimeData.renderInstance.buffer, runtimeData.renderInstance.canvas);

        if (engineConfig.render_ui) {
            engineConfig.render_ui(engineConfig.gameState, runtimeData.renderInstance.canvas);
        }
        canvas_commit(runtimeData.renderInstance.canvas);


        for (i = 0; i < 6; i++) {
            runtimeData.inputState[i] = InputTypeMAX;
        }

        furi_mutex_release(runtimeData.update_mutex);
        furi_thread_yield();
    }

    cleanup_engine();
}
