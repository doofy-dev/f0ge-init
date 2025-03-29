#pragma once
#include <furi.h>
#include "../math/vector.h"
typedef struct Component Component;

enum CamFollowDirection {
    FollowNone=0,
    FollowLeft=1<<1,
    FollowRight=1<<2,
    FollowUp=1<<3,
    FollowDown=1<<4
};

extern Component com_camera_follow;


void cam_follow_set_area(uint8_t left, uint8_t right, uint8_t up, uint8_t down);
void cam_shift(Vector amount);
void cam_follow_set_directions(uint8_t directions);