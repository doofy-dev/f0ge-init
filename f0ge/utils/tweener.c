#include "tweener.h"
#include "helpers.h"

static List *tweeners = NULL;
static RuntimeData *tweener_runtime_data;

void tweener_prepare(RuntimeData *data) {
    if (!tweeners)
        tweeners = list_make();
    tweener_runtime_data = data;
}

void tweener_cleanup() {
    list_free(tweeners);
}

void tweener_start(Tweener *tweener) {
    tweener->t = 0;
    tweener->finished = false;
    tweener->finished = tweener->update(tweener);

    list_push_back(tweener, tweeners);
}

void tweener_update() {
    if (!tweeners) return;

    ListItem *start = tweeners->head;
    while (start) {

        Tweener *data = start->data;
        start = start->next;
        if (data) {
            if (data->delay <= 0) {
                data->t += tweener_runtime_data->delta_time * (1.f / data->length);
                if (data->t > 1) data->t = 1;
                data->finished = data->update(data) || data->t == 1;
                if (data->finished) {
                    data->end(data);
                    list_remove_item(data, tweeners);
                }
            } else {
                data->delay -= tweener_runtime_data->delta_time;
            }
        }
    }
}


void tweener_end(Tweener *tweener) {
    check_pointer(tweener);
    tweener->finished=true;
    tweener->t=1;
    tweener->end(tweener);
    list_remove_item(tweener, tweeners);
}