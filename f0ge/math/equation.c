#include "equation.h"
#include <math.h>

float inverse_tanh(double x) {
    return 0.5f * (float) log((1 + x) / (1 - x));
}

float lerp_number(float a, float b, float t) {
    if (t >= 1) return b;
    if (t <= 0) return a;
    return (1 - t) * a + t * b;
}

float smoothstep(float t) {
    return t*t*(3 - t*2);
}

float clamp(float x, float lowerlimit, float upperlimit) {
    if (x < lowerlimit) return lowerlimit;
    if (x > upperlimit) return upperlimit;
    return x;
}

float scale_range(float value, float old_min, float old_max, float new_min, float new_max) {
    // Avoid division by zero if source range is effectively a single point
    if (old_min == old_max) {
        return new_min; // Return the start of the target range
    }

    // Linearly map the value from [x_min, x_max] to [a, b]
    return new_min + (new_max - new_min) * (value - old_min) / (old_max - old_min);
}