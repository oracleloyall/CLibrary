#ifndef	TIMER_H_
#define	TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <malloc.h>
#include "queue.h"

/* Timer's handler */
typedef struct{
}zx_timer_t;

typedef struct{
}zx_timer_index_t;

typedef void timer_cb_t(void * event);

#define	TM_TYPE_QUEUE 0
#define	TM_TYPE_WHEEL 1

#define MAX_WHEEL_SIZE 10000

zx_timer_t * zx_timer_create(unsigned long wheel_size, int TM_TYPE);

int zx_timer_add(zx_timer_t * timer, unsigned long current_time, unsigned long timeout, \
    timer_cb_t callback, void* event, zx_timer_index_t ** index);

unsigned long zx_timer_del(zx_timer_t * timer, zx_timer_index_t * index, \
    timer_cb_t callback, void * para);

unsigned long zx_timer_check(zx_timer_t * timer, unsigned long current_time, unsigned long max_cb_times);

void zx_timer_destroy(zx_timer_t * timer, timer_cb_t * callback, void * para);

unsigned long zx_timer_count(zx_timer_t * timer);

unsigned long zx_timer_memsize(zx_timer_t * timer);
int zx_timer_reset(zx_timer_t * timer, zx_timer_index_t * index, unsigned long current_time, unsigned long timeout);

#ifdef	__cplusplus
}
#endif

#endif
