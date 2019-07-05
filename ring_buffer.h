/*
 * create 2019/07
 */
#include <pthread.h>
#include <semaphore.h>
#include <sys/param.h>//Function MIN
#define ASSERT_RET(run, cond) assert(run cond)
#ifndef zx_RINGBUFFER_H
#define zx_RINGBUFFER_H
#define zx_RINGBUFFER_BLOCKING 0
#define zx_RINGBUFFER_POLLING 1
typedef struct zx_ringbuffer {
	volatile size_t start;
	size_t size;
	int mode;
	void *volatile*elements;
	pthread_mutex_t wlock;
	pthread_mutex_t rlock;
	pthread_spinlock_t swlock;
	pthread_spinlock_t srlock;
	pthread_mutex_t empty_lock;
	pthread_mutex_t full_lock;
	pthread_cond_t empty_cond; // Signal when empties are ready
	pthread_cond_t full_cond; // Signal when fulls are ready
	volatile size_t end;
} zx_ringbuffer_t;

  int zx_ringbuffer_init(zx_ringbuffer_t * rb, size_t size, int mode);
  void zx_zero_ringbuffer(zx_ringbuffer_t * rb);
  void zx_ringbuffer_destroy(zx_ringbuffer_t * rb);
  int zx_ringbuffer_is_empty(const zx_ringbuffer_t * rb);
  int zx_ringbuffer_is_full(const zx_ringbuffer_t * rb);

  void zx_ringbuffer_write(zx_ringbuffer_t * rb, void* value);
  int zx_ringbuffer_try_write(zx_ringbuffer_t * rb, void* value);
  void zx_ringbuffer_swrite(zx_ringbuffer_t * rb, void* value);
  int zx_ringbuffer_try_swrite(zx_ringbuffer_t * rb, void* value);
  int zx_ringbuffer_try_swrite_bl(zx_ringbuffer_t * rb, void* value);

  void* zx_ringbuffer_read(zx_ringbuffer_t *rb) ;
  int zx_ringbuffer_try_read(zx_ringbuffer_t *rb, void ** value);
  void * zx_ringbuffer_sread(zx_ringbuffer_t *rb);
  int zx_ringbuffer_try_sread(zx_ringbuffer_t *rb, void ** value);
  int zx_ringbuffer_try_sread_bl(zx_ringbuffer_t *rb, void ** value);



  size_t zx_ringbuffer_write_bulk(zx_ringbuffer_t *rb, void *values[], size_t nb_buffers, size_t min_nb_buffers);
  size_t zx_ringbuffer_read_bulk(zx_ringbuffer_t *rb, void *values[], size_t nb_buffers, size_t min_nb_buffers);
  size_t zx_ringbuffer_sread_bulk(zx_ringbuffer_t *rb, void *values[], size_t nb_buffers, size_t min_nb_buffers);
  size_t zx_ringbuffer_swrite_bulk(zx_ringbuffer_t *rb, void *values[], size_t nb_buffers, size_t min_nb_buffers);

#endif
