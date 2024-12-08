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