#include "helpers.h"
#include "list.h"
#include <furi.h>
#include <math.h>

int16_t pointer_count=0;

bool _test_ptr(void *p) {
    return p != NULL;
}

bool _check_ptr(void *p, const char *file, int line, const char *func) {
    UNUSED(file);
    UNUSED(line);
    UNUSED(func);
    if (p == NULL) {
        FURI_LOG_W("App", "[NULLPTR] %s:%s():%i", get_basename((char *) file), func, line);
    }

    return _test_ptr(p);
}

char *get_basename(const char *path) {
    const char *base = path;
    while (*path) {
        if (*path++ == '/') {
            base = path;
        }
    }
    return (char *) base;
}

size_t curr_time() { return DWT->CYCCNT; }

void *_allocate(size_t size, const char *file, int line, const char *func) {
    pointer_count++;
    void *data=malloc(size);
    FURI_LOG_D("Memory", "Allocated %d bytes at %p\tcaller: %s:%s():%i", size, data, get_basename((char *) file), func, line);
    return data;
}

void _release_debug(void *p, const char *file, int line, const char *func) {
    check_pointer(p);
    pointer_count--;
    FURI_LOG_D("Memory", "Releasing %p\tcaller:%s:%s():%i", p, get_basename((char *) file), func, line);

    _release(p);
}

void _release(void *p){
    free(p);
}

void check_leak(){
    if(pointer_count>0){
        FURI_LOG_E("Memory", "Leak detected, pointers left in memory: %d", pointer_count);
    }
}