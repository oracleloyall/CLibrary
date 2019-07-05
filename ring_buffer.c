/*
 * create data 2019/7
 */
#include "ring_buffer.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define LOCK_TYPE_MUTEX 0 // Default if not defined
#define LOCK_TYPE_SPIN 1
#define LOCK_TYPE_NONE 2


#define USE_CHECK_EARLY 1

#define USE_LOCK_TYPE LOCK_TYPE_SPIN
#if USE_LOCK_TYPE == LOCK_TYPE_SPIN
#	define LOCK(dir) ASSERT_RET(pthread_spin_lock(&rb->s ## dir ## lock), == 0)
#	define UNLOCK(dir) ASSERT_RET(pthread_spin_unlock(&rb->s ## dir ## lock), == 0)
#	define TRY_LOCK(dir, action) if(pthread_spin_lock(&rb->s ## dir ## lock) != 0) { \
								action }
#elif USE_LOCK_TYPE == LOCK_TYPE_NONE
#	define LOCK(dir) 
#	define UNLOCK(dir)
#	define TRY_LOCK(dir, action)
#else // Mutex
#	define LOCK(dir) ASSERT_RET(pthread_mutex_lock(&rb-> dir ## lock), == 0)
#	define UNLOCK(dir) ASSERT_RET(pthread_mutex_unlock(&rb-> dir ## lock), == 0)
#	define TRY_LOCK(dir, action) if(pthread_mutex_lock(&rb-> dir ## lock) != 0) {\
								action }
#endif


/**
 * Implements a FIFO queue via a ring buffer.
 * 
 * @param rb A pointer to a ringbuffer structure.
 * @param size The maximum size of the ring buffer.
 * @param mode The mode allows selection to use semaphores to signal when data
 * 				becomes available. zx_RINGBUFFER_BLOCKING or zx_RINGBUFFER_POLLING.
 * @return If successful returns 0 otherwise -1 upon failure.
 */
  int zx_ringbuffer_init(zx_ringbuffer_t * rb, size_t size, int mode) {
	size = size + 1;
	if (!(size > 1))
		return -1;//size
	rb->size = size;
	rb->start = 0;
	rb->end = 0;
	rb->elements = calloc(rb->size, sizeof(void*));
	if (!rb->elements)
		return -2;//calloc error
	rb->mode = mode;
	if (mode == zx_RINGBUFFER_BLOCKING) {
		/* The signaling part - i.e. release when data is ready to read */
		pthread_cond_init(&rb->full_cond, NULL);
		pthread_cond_init(&rb->empty_cond, NULL);
		ASSERT_RET(pthread_mutex_init(&rb->empty_lock, NULL), == 0);
		ASSERT_RET(pthread_mutex_init(&rb->full_lock, NULL), == 0);
	}
	/* The mutual exclusion part */
#if USE_LOCK_TYPE == LOCK_TYPE_SPIN
#warning "using spinners"
	ASSERT_RET(pthread_spin_init(&rb->swlock, 0), == 0);
	ASSERT_RET(pthread_spin_init(&rb->srlock, 0), == 0);
#elif USE_LOCK_TYPE == LOCK_TYPE_NONE
#warning "No locking used"
#else
	ASSERT_RET(pthread_mutex_init(&rb->wlock, NULL), == 0);
	ASSERT_RET(pthread_mutex_init(&rb->rlock, NULL), == 0);
#endif
	return 0;
}

/**
 * Destroys the ring buffer along with any memory allocated to it
 * @param rb The ringbuffer to destroy
 */
  void zx_ringbuffer_destroy(zx_ringbuffer_t * rb) {
#if USE_LOCK_TYPE == LOCK_TYPE_SPIN
	ASSERT_RET(pthread_spin_destroy(&rb->swlock), == 0);
	ASSERT_RET(pthread_spin_destroy(&rb->srlock), == 0);
#elif USE_LOCK_TYPE == LOCK_TYPE_NONE
#endif
	ASSERT_RET(pthread_mutex_destroy(&rb->wlock), == 0);
	ASSERT_RET(pthread_mutex_destroy(&rb->rlock), == 0);
	if (rb->mode == zx_RINGBUFFER_BLOCKING) {
		pthread_cond_destroy(&rb->full_cond);
		pthread_cond_destroy(&rb->empty_cond);
	}
	rb->size = 0;
	rb->start = 0;
	rb->end = 0;
	free((void *)rb->elements);
	rb->elements = NULL;
}

/**
 * Tests to see if ringbuffer is empty, when using multiple threads
 * this doesn't guarantee that the next operation wont block. Use
 * write/read try instead.
 */
  int zx_ringbuffer_is_empty(const zx_ringbuffer_t * rb) {
	return rb->start == rb->end;
}

/**
 * Tests to see if ringbuffer is empty, when using multiple threads
 * this doesn't guarantee that the next operation wont block. Use
 * write/read try instead.
 */
  int zx_ringbuffer_is_full(const zx_ringbuffer_t * rb) {
	return rb->start == ((rb->end + 1) % rb->size);
}

static inline size_t zx_ringbuffer_nb_full(const zx_ringbuffer_t *rb) {
	if (rb->end < rb->start)
		return rb->end + rb->size - rb->start;
	else
		return rb->end - rb->start;
	// return (rb->end + rb->size - rb->start) % rb->size;
}

static inline size_t zx_ringbuffer_nb_empty(const zx_ringbuffer_t *rb) {
	if (rb->start <= rb->end)
		return rb->start + rb->size - rb->end - 1;
	else
		return rb->start - rb->end - 1;
	// return (rb->start + rb->size - rb->end - 1) % rb->size;
}

/**
 * Waits for a empty slot, that we can write to.
 * @param rb The ringbuffer
 */
static inline void wait_for_empty(zx_ringbuffer_t *rb) {
	/* Need an empty to start with */
	if (rb->mode == zx_RINGBUFFER_BLOCKING) {
		pthread_mutex_lock(&rb->empty_lock);
		while (zx_ringbuffer_is_full(rb))
			pthread_cond_wait(&rb->empty_cond, &rb->empty_lock);
		pthread_mutex_unlock(&rb->empty_lock);
	} else {
		while (zx_ringbuffer_is_full(rb))
			/* Yield our time, why?, we tried and failed to write an item
			 * to the buffer - so we should give up our time in the hope
			 * that the reader thread can empty the buffer giving us a good
			 * burst to write without blocking */
			sched_yield();//_mm_pause();
	}
}

/**
 * Waits for a full slot, that we read from.
 * @param rb The ringbuffer
 */
static inline void wait_for_full(zx_ringbuffer_t *rb) {
	/* Need an empty to start with */
	if (rb->mode == zx_RINGBUFFER_BLOCKING) {
		pthread_mutex_lock(&rb->full_lock);
		while (zx_ringbuffer_is_empty(rb))
			pthread_cond_wait(&rb->full_cond, &rb->full_lock);
		pthread_mutex_unlock(&rb->full_lock);
	} else {
		while (zx_ringbuffer_is_empty(rb))
			/* Yield our time, why?, we tried and failed to write an item
			 * to the buffer - so we should give up our time in the hope
			 * that the reader thread can empty the buffer giving us a good
			 * burst to write without blocking */
			sched_yield();//_mm_pause();
	}
}

/**
 * Notifies we have created a full slot, after a write.
 * @param rb The ringbuffer
 */
static inline void notify_full(zx_ringbuffer_t *rb) {
	/* Need an empty to start with */
	if (rb->mode == zx_RINGBUFFER_BLOCKING) {
		pthread_mutex_lock(&rb->full_lock);
		pthread_cond_broadcast(&rb->full_cond);
		pthread_mutex_unlock(&rb->full_lock);
	}
}

/**
 * Notifies we have created an empty slot, after a read.
 * @param rb The ringbuffer
 */
static inline void notify_empty(zx_ringbuffer_t *rb) {
	/* Need an empty to start with */
	if (rb->mode == zx_RINGBUFFER_BLOCKING) {
		pthread_mutex_lock(&rb->empty_lock);
		pthread_cond_broadcast(&rb->empty_cond);
		pthread_mutex_unlock(&rb->empty_lock);
	}
}

/**
 * Performs a blocking write to the buffer, upon return the value will be
 * stored. This will not clobber old values.
 * 
 * This assumes only one thread writing at once. Use 
 * zx_ringbuffer_swrite for a thread safe version.
 * 
 * @param rb a pointer to zx_ringbuffer structure
 * @param value the value to store
 */
  void zx_ringbuffer_write(zx_ringbuffer_t * rb, void* value) {
	/* Need an empty to start with */
	wait_for_empty(rb);
	rb->elements[rb->end] = value;
	rb->end = (rb->end + 1) % rb->size;
	notify_full(rb);
}

/**
 * Performs a blocking write to the buffer, upon return the value will be
 * stored. This will not clobber old values.
 * 
 * This assumes only one thread writing at once. Use 
 * zx_ringbuffer_swrite for a thread safe version.
 *
 * Packets are written out from start to end in order, if only some packets are
 * written those at the end of the array will be still be unwritten.
 *
 * @param rb a pointer to zx_ringbuffer structure
 * @param values A pointer to a memory address read in
 * @param nb_buffer The maximum buffers to write i.e. the length of values
 * @param min_nb_buffers The minimum number of buffers to write
 * @param value the value to store
 */
  size_t zx_ringbuffer_write_bulk(zx_ringbuffer_t * rb, void *values[], size_t nb_buffers, size_t min_nb_buffers) {
	size_t nb_ready;
	size_t i = 0;

	if (min_nb_buffers > nb_buffers) {
		fprintf(stderr, "min_nb_buffers must be greater than or equal to nb_buffers in zx_ringbuffer_write_bulk()\n");
		return ~0U;
	}
	if (!min_nb_buffers && zx_ringbuffer_is_full(rb))
		return 0;

	do {
		register size_t end;
		wait_for_empty(rb);
		nb_ready = zx_ringbuffer_nb_empty(rb);
		nb_ready = MIN(nb_ready, nb_buffers-i);
		nb_ready += i;
		// TODO consider optimising into at most 2 memcpys??
		end = rb->end;
		for (; i < nb_ready; i++) {
			rb->elements[end] = values[i];
			end = (end + 1) % rb->size;
		}
		rb->end = end;
		notify_full(rb);
	} while (i < min_nb_buffers);
	return i;
}

/**
 * Performs a non-blocking write to the buffer, if their is no space
 * or the list is locked by another thread this will return immediately 
 * without writing the value. Assumes that only one thread is writing.
 * Otherwise use zx_ringbuffer_try_swrite.
 * 
 * @param rb a pointer to zx_ringbuffer structure
 * @param value the value to store
 * @return 1 if a object was written otherwise 0.
 */
  int zx_ringbuffer_try_write(zx_ringbuffer_t * rb, void* value) {
	if (zx_ringbuffer_is_full(rb))
		return 0;
	zx_ringbuffer_write(rb, value);
	return 1;
}

/**
 * Waits and reads from the supplied buffer, note this will block forever.
 * 
 * @param rb a pointer to zx_ringbuffer structure
 * @param out a pointer to a memory address where the returned item would be placed
 * @return The object that was read
 */
  void* zx_ringbuffer_read(zx_ringbuffer_t *rb) {
	void* value;
	
	/* We need a full slot */
	wait_for_full(rb);
	value = rb->elements[rb->start];
	rb->start = (rb->start + 1) % rb->size;
	/* Now that's an empty slot */
	notify_empty(rb);
	return value;
}

/**
 * Waits and reads from the supplied buffer, note this will block forever.
 * Attempts to read the requested number of packets, however will return
 * with only the number that are currently ready.
 *
 * Set min_nb_buffers to 0 to 'try' read packets.
 *
 * The buffer is filled from start to finish i.e. if 2 is returned [0] and [1]
 * are valid.
 * 
 * @param rb a pointer to zx_ringbuffer structure
 * @param values A pointer to a memory address where the returned item would be placed
 * @param nb_buffer The maximum buffers to read i.e. the length of values
 * @param min_nb_buffers The minimum number of buffers to read
 * @return The number of packets read
 */
  size_t zx_ringbuffer_read_bulk(zx_ringbuffer_t *rb, void *values[], size_t nb_buffers, size_t min_nb_buffers) {
	size_t nb_ready;
	size_t i = 0;

	if (min_nb_buffers > nb_buffers) {
                fprintf(stderr, "min_nb_buffers must be greater than or equal to nb_buffers in zx_ringbuffer_write_bulk()\n");
                return ~0U;
        }

	if (!min_nb_buffers && zx_ringbuffer_is_empty(rb))
		return 0;

	do {
		register size_t start;
		/* We need a full slot */
		wait_for_full(rb);

		nb_ready = zx_ringbuffer_nb_full(rb);
		nb_ready = MIN(nb_ready, nb_buffers-i);
		// Additional to the i we've already read
		nb_ready += i;
		start = rb->start;
		for (; i < nb_ready; i++) {
			values[i] = rb->elements[start];
			start = (start + 1) % rb->size;
		}
		rb->start = start;
		/* Now that's an empty slot */
		notify_empty(rb);
	} while (i < min_nb_buffers);
	return i;
}


/**
 * Tries to read from the supplied buffer if it fails this and returns
 * 0 to indicate nothing was read.
 * 
 * @param rb a pointer to zx_ringbuffer structure
 * @param out a pointer to a memory address where the returned item would be placed
 * @return 1 if a object was received otherwise 0, in this case out remains unchanged
 */
  int zx_ringbuffer_try_read(zx_ringbuffer_t *rb, void ** value) {
	if (zx_ringbuffer_is_empty(rb))
		return 0;
	*value = zx_ringbuffer_read(rb);
	return 1;
}

/**
 * A thread safe version of zx_ringbuffer_write
 */
  void zx_ringbuffer_swrite(zx_ringbuffer_t * rb, void* value) {
	LOCK(w);
	zx_ringbuffer_write(rb, value);
	UNLOCK(w);
}

/**
 * A thread safe version of zx_ringbuffer_write_bulk
 */
  size_t zx_ringbuffer_swrite_bulk(zx_ringbuffer_t * rb, void *values[], size_t nb_buffers, size_t min_nb_buffers) {
	size_t ret;
#if USE_CHECK_EARLY
	if (!min_nb_buffers && zx_ringbuffer_is_full(rb)) // Check early
		return 0;
#endif
	LOCK(w);
	ret = zx_ringbuffer_write_bulk(rb, values, nb_buffers, min_nb_buffers);
	UNLOCK(w);
	return ret;
}

/**
 * A thread safe version of zx_ringbuffer_try_write
 */
  int zx_ringbuffer_try_swrite(zx_ringbuffer_t * rb, void* value) {
	int ret;
#if USE_CHECK_EARLY
	if (zx_ringbuffer_is_full(rb)) // Check early, drd issues
		return 0;
#endif
	TRY_LOCK(w, return 0;);
	ret = zx_ringbuffer_try_write(rb, value);
	UNLOCK(w);
	return ret;
}

/**
 * A thread safe version of zx_ringbuffer_try_write
 * Unlike zx_ringbuffer_try_swrite this will block on da lock just
 * not the data. This will block for a long period of time if zx_ringbuffer_sread
 * is holding the lock. However will not block for long if only zx_ringbuffer_try_swrite_bl
 * and zx_ringbuffer_try_swrite are being used.
 */
  int zx_ringbuffer_try_swrite_bl(zx_ringbuffer_t * rb, void* value) {
	int ret;
#if USE_CHECK_EARLY
	if (zx_ringbuffer_is_full(rb)) // Check early
		return 0;
#endif
	LOCK(w);
	ret = zx_ringbuffer_try_write(rb, value);
	UNLOCK(w);
	return ret;
}

/**
 * A thread safe version of zx_ringbuffer_read
 */
  void * zx_ringbuffer_sread(zx_ringbuffer_t *rb) {
	void* value;
	LOCK(r);
	value = zx_ringbuffer_read(rb);
	UNLOCK(r);
	return value;
}

/**
 * A thread safe version of zx_ringbuffer_read_bulk
 */
  size_t zx_ringbuffer_sread_bulk(zx_ringbuffer_t * rb, void *values[], size_t nb_buffers, size_t min_nb_buffers) {
	size_t ret;
#if USE_CHECK_EARLY
	if (!min_nb_buffers && zx_ringbuffer_is_empty(rb)) // Check early
		return 0;
#endif
	LOCK(r);
	ret = zx_ringbuffer_read_bulk(rb, values, nb_buffers, min_nb_buffers);
	UNLOCK(r);
	return ret;
}

/**
 * A thread safe version of zx_ringbuffer_try_write
 */
  int zx_ringbuffer_try_sread(zx_ringbuffer_t *rb, void ** value) {
	int ret;
#if USE_CHECK_EARLY
	if (zx_ringbuffer_is_empty(rb)) // Check early
		return 0;
#endif
	TRY_LOCK(r, return 0;);
	ret = zx_ringbuffer_try_read(rb, value);
	UNLOCK(r);
	return ret;
}

/**
 * A thread safe version of zx_ringbuffer_try_wread
 * Unlike zx_ringbuffer_try_sread this will block on da lock just
 * not the data. This will block for a long period of time if zx_ringbuffer_sread
 * is holding the lock. However will not block for long if only zx_ringbuffer_try_sread_bl
 * and zx_ringbuffer_try_sread are being used.
 */
  int zx_ringbuffer_try_sread_bl(zx_ringbuffer_t *rb, void ** value) {
	int ret;
#if USE_CHECK_EARLY
	if (zx_ringbuffer_is_empty(rb)) // Check early
		return 0;
#endif
	LOCK(r);
	ret = zx_ringbuffer_try_read(rb, value);
	UNLOCK(r);
	return ret;
}

  void zx_zero_ringbuffer(zx_ringbuffer_t * rb)
{
	rb->start = 0;
	rb->end = 0;
	rb->size = 0;
	rb->elements = NULL;
}


