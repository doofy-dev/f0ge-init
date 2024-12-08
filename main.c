#include "f0ge/f0ge.h"
#include "f0ge/utils/list.h"
#include "rendertest_icons.h"
#include "f0ge/utils/helpers.h"

typedef struct {
    int a;
    int b;
} ASD;

Buffer *image;

void render_node(Node *self, Buffer *buffer) {
    UNUSED(self);
    Vector pos = {0, 0};
    buffer_draw_all(buffer, image, &pos);
}

void comp_start(Component *self) {
    UNUSED(self);
}

float direction = 1;

void comp_upd(Component *self, float delta) {
    self->node->transform.rotation += 40.0f * delta;
    self->node->transform.scale.x += direction / 2 * delta;
    if (self->node->transform.scale.x > 2) direction = -1;
    if (self->node->transform.scale.x < 0.01) direction = 1;
    if (self->node->transform.rotation > 360) self->node->transform.rotation = 0;
    self->node->transform.dirty = true;
}

void comp2_upd(Component *self, float delta) {
    self->node->transform.rotation += 40.0f * delta;
    if (self->node->transform.rotation > 360) self->node->transform.rotation = 0;
    self->node->transform.dirty = true;
}

void comp_end(Component *self) {
    UNUSED(self);
}

int main() {
    image = buffer_decompress_icon(&I_extra_ball);

    Component maincomp = {
        .start = &comp_start,
        .update = &comp_upd,
        .end = &comp_end,
    };
    Component maincomp2 = {
        .start = &comp_start,
        .update = &comp2_upd,
        .end = &comp_end,
    };
    Node child = {
        .transform = {
            .position = {50, 0},
            .rotation = 0,
            .scale = {1, 1}
        },
        .children = NULL,
        .components = list_from(1, &maincomp),
        .render = &render_node,
    };
    Node root = {
        .transform = {
            .position = {64, 32},
            .rotation = 0,
            .scale = {0.5f, 0.5f}
        },
        .children = list_from(1, &child),
        .components = list_from(1, &maincomp2),
        .render = &render_node,
    };

    init_engine((EngineConfig){
        .muted = false,
        .backlight = true
    });
    set_scene(&root);
    start_loop();
    buffer_release(image);
}

int32_t render_app(void *p) {
    UNUSED(p);
    return main();
}
