#include "cam_utils.h"
#include "../node.h"
#include "../graphics/render.h"
#include "../component.h"

void cam_follow_update(Node *self, float delta);

static uint8_t follow_directions = FollowNone;
static uint8_t edge_left = 0, edge_right = 0, edge_up = 0, edge_down = 0;
Vector center = {-64, -32};
Component com_camera_follow = {
    .start = NULL,
    .end = NULL,
    .update = &cam_follow_update,
};

void cam_follow_update(Node *self, float delta) {
    UNUSED(delta);

    if (follow_directions == FollowNone) return;

    Vector pos;
    matrix_get_translation(&(self->transform.transformation_matrix), &pos);

    //shift coordinates to center
    vector_add(&center, &pos, &pos);

    //if all side is half the screen, just set the position to the node position
    if (edge_left == 64 || edge_right == 64 || edge_up == 32 || edge_down == 32) {
        set_camera(pos);
    }
    else {
        Vector current_cam_pos = get_camera();
        //based on edge config and follow mask move the camera
        if (follow_directions & FollowLeft && pos.x < (current_cam_pos.x - edge_left)) {
            current_cam_pos.x += (pos.x - current_cam_pos.x + edge_left);
        }
        else if (follow_directions & FollowRight && pos.x > (current_cam_pos.x + edge_right)) {
            current_cam_pos.x += (pos.x - current_cam_pos.x - edge_right);
        }
        if (follow_directions & FollowUp && pos.y < (current_cam_pos.y - edge_up)) {
            current_cam_pos.y += (pos.y - current_cam_pos.y + edge_up);
        }
        else if (follow_directions & FollowDown && pos.y > (current_cam_pos.y + edge_down)) {
            current_cam_pos.y += (pos.y - current_cam_pos.y - edge_down);
        }
        set_camera(current_cam_pos);
    }
}
void cam_shift(Vector amount) {
    center = (Vector){-amount.x, -amount.y};
}
void cam_follow_set_area(uint8_t left, uint8_t right, uint8_t up, uint8_t down) {
    edge_left = left;
    edge_right = right;
    edge_up = up;
    edge_down = down;
}

void cam_follow_set_directions(uint8_t directions) {
    follow_directions = directions;
}
