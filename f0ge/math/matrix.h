#pragma once

#include "vector.h"
#define IDENTITY_MATRIX {1,0,0, 0,1,0, 0,0,1}

typedef float Matrix[9];
typedef struct Transform Transform;
struct Transform{
    bool dirty;
    Vector scale;
    Vector position;
    float rotation;

    Matrix transformation_matrix;
};


void matrix_scale(Vector *scale, Matrix *target);
void matrix_reset(Matrix *target);

void matrix_translate(Vector *translate, Matrix *target);
void matrix_copy(Matrix *source, Matrix *target);

void matrix_rotate(float rotation, Matrix *target);

void matrix_mul(Matrix *a, Matrix *b, Matrix *target);

void matrix_mul_vector(Matrix *a, Vector *vector, Vector *target);

void matrix_get_translation(Matrix *a, Vector *translation);

float matrix_get_rotation(Matrix *a);
void matrix_get_scaling(Matrix *a, Vector *target);

void compute_transformation_matrix(Transform* target, Transform *parent);

void matrix_print(Matrix *a);