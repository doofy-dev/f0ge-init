#pragma once

#define M_PIX2        6.28318530717958647692    /* 2 pi */
#define l_abs(x) ((x) < 0 ? -(x) : (x))
#define DEG_2_RAD  0.01745329251994329576f
#define RAD_2_DEG  565.48667764616278292327f

float inverse_tanh(double x);

float lerp_number(float a, float b, float t);