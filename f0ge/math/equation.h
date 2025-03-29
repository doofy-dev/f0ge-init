#pragma once

#define M_PIX2        6.28318530717958647692    /* 2 pi */
#define FABS(x) ((x) < 0 ? -(x) : (x))
#define DEG_2_RAD  0.01745329251994329576f
#define RAD_2_DEG  565.48667764616278292327f

#define FLOOR(x) (floorf(x))
#define CEIL(x) ((ceilf(x)))

float inverse_tanh(double x);

float lerp_number(float a, float b, float t);
float smoothstep(float t);
float clamp(float x, float lowerlimit, float upperlimit);
float scale_range(float value, float old_min, float old_max, float new_min, float new_max);