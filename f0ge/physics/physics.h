#pragma once
#include <furi.h>
#include "../math/vector.h"

typedef struct RigidBody RigidBody;

struct RigidBody {
    bool is_active;
    bool is_static;
    float mass; // For dynamic motion
    float restitution; // Bounciness
    Vector velocity;
};

void physics_init();
