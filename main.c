#include "f0ge/f0ge.h"
#include "rendertest_icons.h"
#include "f0ge/components/cam_utils.h"
#include "f0ge/graphics/asset.h"
#include "f0ge/graphics/render.h"

typedef struct {
    Vector momentum;
    float speed;
    float MAX_SPEED;
    float ACC;
    Vector prevCenter;
} CarData;

void car_upd(Node *self, float delta, void *data) {
    CarData *car = (CarData *) data;
    Vector forward;
    matrix_forward(&(self->transform.transformation_matrix), &forward);

    vector_normalized(&forward, &forward);

    if (is_down(InputKeyUp)) {
        if (car->speed < -1) car->speed /= 3;
        else {
            car->speed += (car->MAX_SPEED * car->ACC) * delta;
        }
    } else if (is_down(InputKeyDown)) {
        if (car->speed > 1) {
            car->speed /= 2;
        } else
            car->speed -= (car->MAX_SPEED * car->ACC) * delta;
    } else {
        car->speed *= 0.8f;
    }


    if (car->speed > car->MAX_SPEED) {
        car->speed = car->MAX_SPEED;
    }
    if (car->speed < -car->MAX_SPEED) {
        car->speed = -car->MAX_SPEED;
    }

    car->momentum.x = car->speed * delta * forward.y;
    car->momentum.y = car->speed * delta * -forward.x;

    float l = MIN(vector_length_sqrt(&(car->momentum)), 10);

    if (car->speed != 0) {
        int8_t sign = 1;
        if (car->speed < 0) sign = -1;
        if (is_down(InputKeyLeft)) sign *= -1;

        if (is_down(InputKeyLeft) || is_down(InputKeyRight)) {
            self->transform.rotation += sign * delta * 10.f * l;
        }
    }

    vector_add(&(self->transform.position), &(car->momentum), &(self->transform.position));


    vector_normalized(&(car->momentum), &forward);
    forward.x *= l * -6.4f;
    forward.y *= l * -3.2f;
    Vector center = {64 + forward.x, 32 + forward.y};
    vector_lerp(&(car->prevCenter), &center, 0.2f, &center);

    //move the camera follower component's center to be able to see ahead
    cam_shift(center);

    (car->prevCenter) = center;

    self->transform.dirty = true;
}

int main() {
    //Set up the renderer for the sprites them
    RenderData car_render = (RenderData){
        .poly = RECTANGLE(-12, -18, 12, 18),
        .tile_mode = TILE_NONE,
        .color = COLOR_BLACK,
        .mask_color = COLOR_WHITE,
        .sprite = asset_get_icon(&I_car),
        .mask = asset_get_icon(&I_car_fill),
    };

    RenderData brick_render = (RenderData){
        .color = COLOR_BLACK,
        .poly = RECTANGLE(0, 0, 16, 16),
        .mask = NULL,
        .tile_mode = TILE_BOTH,
        .sprite = asset_get_icon(&I_block)
    };

    //Set up the player follower camera
    cam_follow_set_directions(FollowLeft | FollowRight | FollowUp | FollowDown);
    cam_follow_set_area(10, 10, 5, 5);

    CarData car_data = {
        .MAX_SPEED = 100,
        .momentum = VECTOR_ZERO,
        .speed = 0,
        .ACC = 0.3f,
        .prevCenter = {64, 32}
    };
    //Create a new component for the driving
    Component maincomp = MAKE_COMPONENT();
    maincomp.update = &car_upd;
    maincomp.data = &car_data;

    CarData car2_data = {
        .MAX_SPEED = 100,
        .momentum = VECTOR_ZERO,
        .speed = 0,
        .ACC = 0.5f,
        .prevCenter = {64, 32}
    };
    //Create a new component for the driving
    Component maincomp2 = MAKE_COMPONENT();
    maincomp2.update = &car_upd;
    maincomp2.data = &car2_data;

    //---------------------------------------------------------------------------------------------
    //Set up the car node
    Node player = MAKE_NODE();
    player.sprite = &car_render;
    add_component(&player, &maincomp);
    add_component(&player, &com_camera_follow);
    player.transform.position = (Vector){64, 34};
    // player.transform.scale = (Vector){2.5f, 2.5f};

    Node player2 = MAKE_NODE();
    player2.sprite = &car_render;
    add_component(&player2, &maincomp2);
    player2.transform.position = (Vector){32, 34};
    // player2.transform.scale = (Vector){2.5f, 2.5f};


    //Set up the brick node
    Node brick = MAKE_NODE();
    brick.sprite = &brick_render;
    brick.transform.position = (Vector){0, 60};
    brick.transform.scale = (Vector){10, 10};


    //Set up the root node that holds all together
    Node root = MAKE_NODE();
    add_child(&root, &brick);
    add_child(&root, &player);
    add_child(&root, &player2);


    //Start the engine
    init_engine((EngineConfig){
        .muted = false,
        .render_fps = 30,
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
