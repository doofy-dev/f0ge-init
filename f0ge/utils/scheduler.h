#pragma once
#include "../f0ge.h"

typedef struct{
    void (*callback)(void *data, RuntimeData *sceneData);
    float t;
    float timeout;
    bool repeat;
    void *data;
} Scheduler;

void scheduler_prepare(RuntimeData *sceneData);
void scheduler_cleanup();
void scheduler_start(Scheduler *scheduler);
void scheduler_update();
void scheduler_stop(Scheduler *scheduler);
void scheduler_stop_all();