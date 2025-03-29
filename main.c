#include "f0ge/f0ge.h"
#include "f0ge/utils/list.h"
#include "rendertest_icons.h"
#include "f0ge/components/cam_utils.h"
#include "f0ge/graphics/asset.h"
#include "f0ge/graphics/render.h"

Vector momentum = VECTOR_ZERO;
float speed = 0;
float MAX_SPEED = 100.f;
float ACC = 0.3f;
Vector prevCenter = {64, 32};


void car_upd(Node *self, float delta) {
    Vector forward;
    matrix_forward(&(self->transform.transformation_matrix), &forward);

    vector_normalized(&forward, &forward);

    if (is_down(InputKeyUp)) {
        if (speed < -1) speed /= 3;
        else {
            speed += (MAX_SPEED * ACC) * delta;
        }
    } else if (is_down(InputKeyDown)) {
        if (speed > 1) {
            speed /= 2;
        } else
            speed -= (MAX_SPEED * ACC) * delta;
    } else {
        speed *= 0.8f;
    }


    if (speed > MAX_SPEED) {
        speed = MAX_SPEED;
    }
    if (speed < -MAX_SPEED) {
        speed = -MAX_SPEED;
    }

    momentum.x = speed * delta * forward.y;
    momentum.y = speed * delta * -forward.x;

    float l = MIN(vector_length_sqrt(&momentum), 10);

    if (speed != 0) {
        int8_t sign = 1;
        if (speed < 0) sign = -1;
        if (is_down(InputKeyLeft)) sign *= -1;

        if (is_down(InputKeyLeft) || is_down(InputKeyRight)) {
            self->transform.rotation += sign * delta * 10.f * l;
        }
    }

    vector_add(&(self->transform.position), &momentum, &(self->transform.position));


    vector_normalized(&momentum, &forward);
    forward.x *= l * -6.4f;
    forward.y *= l * -3.2f;
    Vector center = {64 + forward.x, 32 + forward.y};
    vector_lerp(&prevCenter, &center, 0.2f, &center);

    //move the camera follower component's center to be able to see ahead
    cam_shift(center);

    prevCenter = center;

    self->transform.dirty = true;
}

int main() {
    //Set up the renderer for the sprites them
    RenderData car_render = (RenderData){
        .poly = RECTANGLE(-12, -18, 12, 18),
        .tile_mode = TILE_NONE,
        .color = Black,
        .mask_color = White,
        .sprite = asset_get_icon(&I_car),
        .mask = asset_get_icon(&I_car_fill)
    };

    RenderData brick_render = (RenderData){
        .color = Black,
        .poly = RECTANGLE(0, 0, 16, 16),
        .mask = NULL,
        .tile_mode = TILE_BOTH,
        .sprite = asset_get_icon(&I_block)
    };

    //Set up the player follower camera
    cam_follow_set_directions(FollowLeft | FollowRight | FollowUp | FollowDown);
    cam_follow_set_area(10, 10, 5, 5);

    //Create a new component for the driving
    Component maincomp = {
        .start = NULL,
        .update = &car_upd,
        .end = NULL,
    };

    //Set up the car node
    Node player = {
        .transform = {
            .position = {64, 34},
            .rotation = 45,
            .scale = {1, 1}
        },
        .children = NULL,
        .components = list_from(2, &maincomp, &com_camera_follow),
        .render = NULL,
        .sprite = &car_render,
    };

    //Set up the brick node
    Node brick = {
        .transform = {
            .position = {0, 60},
            .rotation = 0,
            .scale = {10, 10}
        },
        .children = NULL,
        .components = NULL,
        .sprite = &brick_render,
        .render = NULL,
    };

    //Set up the root node that holds all together
    Node root = {
        .transform = {
            .position = {0, 0},
            .rotation = 0,
            .scale = {1, 1}
        },
        .children = list_from(2, &brick, &player),
        .components = NULL,
        .sprite = NULL,
        .render = NULL,
    };

    //Start the engine
    init_engine((EngineConfig){
        .muted = false,
        .backlight = true,
        .render_ui = NULL
    });

    //Set the scene to render
    set_scene(&root);
    start_loop();
}

int32_t render_app(void *p) {
    UNUSED(p);
    return main();
}
