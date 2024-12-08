#include "scheduler.h"
#include "list.h"

static List *schedules = NULL;
static RuntimeData *scheduler_runtime_data;

void scheduler_prepare(RuntimeData *sceneData) {
    if (!schedules) {
        schedules = list_make();
    }
    scheduler_runtime_data = sceneData;
}

void scheduler_cleanup() {
    list_free(schedules);
}

void scheduler_start(Scheduler *scheduler) {
    scheduler->t = 0;
    list_push_back(scheduler, schedules);
}

void scheduler_update() {
    if (!schedules) return;

    ListItem *start = schedules->head;
    while (start) {
        Scheduler *current_scheduler = (Scheduler *) start->data;
        start = start->next;

        current_scheduler->t += scheduler_runtime_data->delta_time;

        if (current_scheduler->t >= current_scheduler->timeout) {
            current_scheduler->t = 0;
            current_scheduler->callback(current_scheduler->data, scheduler_runtime_data);
            if (!current_scheduler->repeat) {
                list_remove_item(current_scheduler, schedules);
            }
        }

    }
}

void scheduler_stop(Scheduler *scheduler) {
    if (!schedules) return;

    ListItem *start = schedules->head;
    while (start) {
        Scheduler *current_scheduler = (Scheduler *) start->data;
        if (scheduler == current_scheduler) {
            list_remove_item(current_scheduler, schedules);
        }

    }
}

void scheduler_stop_all() {
    list_clear(schedules);
}