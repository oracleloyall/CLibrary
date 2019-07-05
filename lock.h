/*
 * Create date:2019 7/3
 * author zhaoxi
 * for automic operator ,such as i386 ,x86...
 */

#ifndef LOCK_H
#define LOCK_H
#define HAVE_SPINLOCKS
#ifdef HAVE_SPINLOCKS	/* skip spinlocks if requested */
#ifdef HAVE_FUNCNAME__FUNC
#define PG_FUNCNAME_MACRO	__func__
#else
#ifdef HAVE_FUNCNAME__FUNCTION
#define PG_FUNCNAME_MACRO	__FUNCTION__
#else
#define PG_FUNCNAME_MACRO	NULL
#endif
#endif

#if defined(__GNUC__) || defined(__INTEL_COMPILER)

#ifdef __i386__		/* 32-bit i386 */
#define HAS_TEST_AND_SET

typedef unsigned char slock_t;

#define TAS(lock) tas(lock)

static __inline__ int
tas(volatile slock_t *lock)
{
	register slock_t _res = 1;
	__asm__ __volatile__(
			"	cmpb	$0,%1	\n"
			"	jne		1f		\n"
			"	lock			\n"
			"	xchgb	%0,%1	\n"
			"1: \n"
			: "+q"(_res), "+m"(*lock)
			: /* no inputs */
			: "memory", "cc");
	return (int) _res;
}

#define SPIN_DELAY() spin_delay()

static __inline__ void
spin_delay(void)
{
	__asm__ __volatile__(
			" rep; nop			\n");
}

#endif	 /* __i386__ */

#ifdef __x86_64__
#define HAS_TEST_AND_SET

typedef unsigned char slock_t;

#define TAS(lock) tas(lock)

#define TAS_SPIN(lock)    (*(lock) ? 1 : TAS(lock))

static __inline__ int
tas(volatile slock_t *lock)
{
	register slock_t _res = 1;

	__asm__ __volatile__(
			"	lock			\n"
			"	xchgb	%0,%1	\n"
			: "+q"(_res), "+m"(*lock)
			: /* no inputs */
			: "memory", "cc");
	return (int) _res;
}

#define SPIN_DELAY() spin_delay()

static __inline__ void
spin_delay(void)
{

	__asm__ __volatile__(
			" rep; nop			\n");
}

#endif	 /* __x86_64__ */

#if defined(__ia64__) || defined(__ia64)

#define HAS_TEST_AND_SET

typedef unsigned int slock_t;

#define TAS(lock) tas(lock)

#define TAS_SPIN(lock)	(*(lock) ? 1 : TAS(lock))

#ifndef __INTEL_COMPILER

static __inline__ int
tas(volatile slock_t *lock)
{
	long int ret;

	__asm__ __volatile__(
			"	xchg4 	%0=%1,%2	\n"
			: "=r"(ret), "+m"(*lock)
			: "r"(1)
			: "memory");
	return (int) ret;
}

#else

static __inline__ int
tas(volatile slock_t *lock)
{
	int ret;

	ret = _InterlockedExchange(lock,1);

	return ret;
}

#define S_UNLOCK(lock)	\
	do { __memory_barrier(); *(lock) = 0; } while (0)

#endif
#endif	 /* __ia64__ || __ia64 */


#if defined(__arm__) || defined(__arm) || defined(__aarch64__) || defined(__aarch64)
#ifdef HAVE_GCC__SYNC_INT32_TAS
#define HAS_TEST_AND_SET

#define TAS(lock) tas(lock)

typedef int slock_t;

static __inline__ int
tas(volatile slock_t *lock)
{
	return __sync_lock_test_and_set(lock, 1);
}

#define S_UNLOCK(lock) __sync_lock_release(lock)

#endif
#endif	 /* __arm__ || __arm || __aarch64__ || __aarch64 */


#if defined(__s390__) || defined(__s390x__)
#define HAS_TEST_AND_SET

typedef unsigned int slock_t;

#define TAS(lock)	   tas(lock)

static __inline__ int
tas(volatile slock_t *lock)
{
	int _res = 0;

	__asm__ __volatile__(
			"	cs 	%0,%3,0(%2)		\n"
			: "+d"(_res), "+m"(*lock)
			: "a"(lock), "d"(1)
			: "memory", "cc");
	return _res;
}

#endif

#if defined(__sparc__)

#define HAS_TEST_AND_SET

typedef unsigned char slock_t;

#define TAS(lock) tas(lock)

static __inline__ int
tas(volatile slock_t *lock)
{
	register slock_t _res;


	__asm__ __volatile__(
			"	ldstub	[%2], %0	\n"
			: "=r"(_res), "+m"(*lock)
			: "r"(lock)
			: "memory");
#if defined(__sparcv7) || defined(__sparc_v7__)

#elif defined(__sparcv8) || defined(__sparc_v8__)
		__asm__ __volatile__ ("stbar	 \n":::"memory");
#else

	__asm__ __volatile__ ("membar #LoadStore | #LoadLoad \n":::"memory");
#endif
	return (int) _res;
}

#if defined(__sparcv7) || defined(__sparc_v7__)

#elif defined(__sparcv8) || defined(__sparc_v8__)

#define S_UNLOCK(lock)	\
do \
{ \
	__asm__ __volatile__ ("stbar	 \n":::"memory"); \
	*((volatile slock_t *) (lock)) = 0; \
} while (0)
#else

#define S_UNLOCK(lock)	\
do \
{ \
	__asm__ __volatile__ ("membar #LoadStore | #StoreStore \n":::"memory"); \
	*((volatile slock_t *) (lock)) = 0; \
} while (0)
#endif

#endif	 /* __sparc__ */

/* PowerPC */
#if defined(__ppc__) || defined(__powerpc__) || defined(__ppc64__) || defined(__powerpc64__)
#define HAS_TEST_AND_SET

typedef unsigned int slock_t;

#define TAS(lock) tas(lock)


#define TAS_SPIN(lock)	(*(lock) ? 1 : TAS(lock))


static __inline__ int
tas(volatile slock_t *lock)
{
	slock_t _t;
	int _res;

	__asm__ __volatile__(
#ifdef USE_PPC_LWARX_MUTEX_HINT
			"	lwarx   %0,0,%3,1	\n"
#else
			"	lwarx   %0,0,%3		\n"
#endif
			"	cmpwi   %0,0		\n"
			"	bne     $+16		\n" /* branch to li %1,1 */
			"	addi    %0,%0,1		\n"
			"	stwcx.  %0,0,%3		\n"
			"	beq     $+12		\n" /* branch to lwsync/isync */
			"	li      %1,1		\n"
			"	b       $+12		\n" /* branch to end of asm sequence */
#ifdef USE_PPC_LWSYNC
			"	lwsync				\n"
#else
			"	isync				\n"
#endif
			"	li      %1,0		\n"

			: "=&r"(_t), "=r"(_res), "+m"(*lock)
			: "r"(lock)
			: "memory", "cc");
	return _res;
}


#ifdef USE_PPC_LWSYNC
#define S_UNLOCK(lock)	\
do \
{ \
	__asm__ __volatile__ ("	lwsync \n" ::: "memory"); \
	*((volatile slock_t *) (lock)) = 0; \
} while (0)
#else
#define S_UNLOCK(lock)	\
do \
{ \
	__asm__ __volatile__ ("	sync \n" ::: "memory"); \
	*((volatile slock_t *) (lock)) = 0; \
} while (0)
#endif /* USE_PPC_LWSYNC */

#endif /* powerpc */

/* Linux Motorola 68k */
#if (defined(__mc68000__) || defined(__m68k__)) && defined(__linux__)
#define HAS_TEST_AND_SET

typedef unsigned char slock_t;

#define TAS(lock) tas(lock)

static __inline__ int
tas(volatile slock_t *lock)
{
	register int rv;

	__asm__ __volatile__(
			"	clrl	%0		\n"
			"	tas		%1		\n"
			"	sne		%0		\n"
			: "=d"(rv), "+m"(*lock)
			: /* no inputs */
			: "memory", "cc");
	return rv;
}

#endif	 /* (__mc68000__ || __m68k__) && __linux__ */


#if defined(__vax__)
#define HAS_TEST_AND_SET

typedef unsigned char slock_t;

#define TAS(lock) tas(lock)

static __inline__ int
tas(volatile slock_t *lock)
{
	register int _res;

	__asm__ __volatile__(
			"	movl 	$1, %0			\n"
			"	bbssi	$0, (%2), 1f	\n"
			"	clrl	%0				\n"
			"1: \n"
			: "=&r"(_res), "+m"(*lock)
			: "r"(lock)
			: "memory");
	return _res;
}

#endif	 /* __vax__ */

#if defined(__mips__) && !defined(__sgi)	/* non-SGI MIPS */
/* Note: on SGI we use the OS' mutex ABI, see below */
/* Note: R10000 processors require a separate SYNC */
#define HAS_TEST_AND_SET

typedef unsigned int slock_t;

#define TAS(lock) tas(lock)

static __inline__ int
tas(volatile slock_t *lock)
{
	register volatile slock_t *_l = lock;
	register int _res;
	register int _tmp;

	__asm__ __volatile__(
			"       .set push           \n"
			"       .set mips2          \n"
			"       .set noreorder      \n"
			"       .set nomacro        \n"
			"       ll      %0, %2      \n"
			"       or      %1, %0, 1   \n"
			"       sc      %1, %2      \n"
			"       xori    %1, 1       \n"
			"       or      %0, %0, %1  \n"
			"       sync                \n"
			"       .set pop              "
			: "=&r" (_res), "=&r" (_tmp), "+R" (*_l)
			: /* no inputs */
			: "memory");
	return _res;
}

/* MIPS S_UNLOCK is almost standard but requires a "sync" instruction */
#define S_UNLOCK(lock)	\
do \
{ \
	__asm__ __volatile__( \
		"       .set push           \n" \
		"       .set mips2          \n" \
		"       .set noreorder      \n" \
		"       .set nomacro        \n" \
		"       sync                \n" \
		"       .set pop              " \
:		/* no outputs */ \
:		/* no inputs */	\
:		"memory"); \
	*((volatile slock_t *) (lock)) = 0; \
} while (0)

#endif /* __mips__ && !__sgi */

#if defined(__m32r__) && defined(HAVE_SYS_TAS_H)	/* Renesas' M32R */
#define HAS_TEST_AND_SET

#include <sys/tas.h>

typedef int slock_t;

#define TAS(lock) tas(lock)

#endif /* __m32r__ */

#if defined(__sh__)				/* Renesas' SuperH */
#define HAS_TEST_AND_SET

typedef unsigned char slock_t;

#define TAS(lock) tas(lock)

static __inline__ int
tas(volatile slock_t *lock)
{
	register int _res;


	__asm__ __volatile__(
			"	tas.b @%2    \n"
			"	movt  %0     \n"
			"	xor   #1,%0  \n"
			: "=z"(_res), "+m"(*lock)
			: "r"(lock)
			: "memory", "t");
	return _res;
}

#endif	 /* __sh__ */

/* These live in s_lock.c, but only for gcc */

#if defined(__m68k__) && !defined(__linux__)	/* non-Linux Motorola 68k */
#define HAS_TEST_AND_SET

typedef unsigned char slock_t;
#endif


#if !defined(S_UNLOCK)
#define S_UNLOCK(lock)	\
	do { __asm__ __volatile__("" : : : "memory");  *(lock) = 0; } while (0)
#endif

#endif	/* defined(__GNUC__) || defined(__INTEL_COMPILER) */


#if !defined(HAS_TEST_AND_SET)	/* We didn't trigger above, let's try here */

#if defined(__hppa) || defined(__hppa__)	/* HP PA-RISC, GCC and HP compilers */

#define HAS_TEST_AND_SET

typedef struct
{
	int sema[4];
}slock_t;

#define TAS_ACTIVE_WORD(lock)	((volatile int *) (((uintptr_t) (lock) + 15) & ~15))

#if defined(__GNUC__)

static __inline__ int
tas(volatile slock_t *lock)
{
	volatile int *lockword = TAS_ACTIVE_WORD(lock);
	register int lockval;

	__asm__ __volatile__(
			"	ldcwx	0(0,%2),%0	\n"
			: "=r"(lockval), "+m"(*lockword)
			: "r"(lockword)
			: "memory");
	return (lockval == 0);
}


#ifdef S_UNLOCK
#undef S_UNLOCK
#endif
#define S_UNLOCK(lock)	\
	do { \
		__asm__ __volatile__("" : : : "memory"); \
		*TAS_ACTIVE_WORD(lock) = -1; \
	} while (0)

#endif /* __GNUC__ */

#define S_INIT_LOCK(lock) \
	do { \
		volatile slock_t *lock_ = (lock); \
		lock_->sema[0] = -1; \
		lock_->sema[1] = -1; \
		lock_->sema[2] = -1; \
		lock_->sema[3] = -1; \
	} while (0)

#define S_LOCK_FREE(lock)	(*TAS_ACTIVE_WORD(lock) != 0)

#endif	 /* __hppa || __hppa__ */

#if defined(__hpux) && defined(__ia64) && !defined(__GNUC__)

#define HAS_TEST_AND_SET

typedef unsigned int slock_t;

#include <ia64/sys/inline.h>
#define TAS(lock) _Asm_xchg(_SZ_W, lock, 1, _LDHINT_NONE)
/* On IA64, it's a win to use a non-locking test before the xchg proper */
#define TAS_SPIN(lock)	(*(lock) ? 1 : TAS(lock))
#define S_UNLOCK(lock)	\
	do { _Asm_mf(); (*(lock)) = 0; } while (0)

#endif	/* HPUX on IA64, non gcc/icc */

#if defined(_AIX)	/* AIX */
/*
 * AIX (POWER)
 */
#define HAS_TEST_AND_SET

#include <sys/atomic_op.h>

typedef int slock_t;

#define TAS(lock)			_check_lock((slock_t *) (lock), 0, 1)
#define S_UNLOCK(lock)		_clear_lock((slock_t *) (lock), 0)
#endif	 /* _AIX */

/* These are in sunstudio_(sparc|x86).s */

#if defined(__SUNPRO_C) && (defined(__i386) || defined(__x86_64__) || defined(__sparc__) || defined(__sparc))
#define HAS_TEST_AND_SET

#if defined(__i386) || defined(__x86_64__) || defined(__sparcv9) || defined(__sparcv8plus)
typedef unsigned int slock_t;
#else
typedef unsigned char slock_t;
#endif

extern slock_t pg_atomic_cas(volatile slock_t *lock, slock_t with,
		slock_t cmp);

#define TAS(a) (pg_atomic_cas((a), 1, 0) != 0)
#endif

#ifdef _MSC_VER
typedef LONG slock_t;

#define HAS_TEST_AND_SET
#define TAS(lock) (InterlockedCompareExchange(lock, 1, 0))

#define SPIN_DELAY() spin_delay()

/* If using Visual C++ on Win64, inline assembly is unavailable.
 * Use a _mm_pause intrinsic instead of rep nop.
 */
#if defined(_WIN64)
static __forceinline void
spin_delay(void)
{
	_mm_pause();
}
#else
static __forceinline void
spin_delay(void)
{
	/* See comment for gcc code. Same code, MASM syntax */
	__asm rep nop;
}
#endif

#include <intrin.h>
#pragma intrinsic(_ReadWriteBarrier)

#define S_UNLOCK(lock)	\
	do { _ReadWriteBarrier(); (*(lock)) = 0; } while (0)

#endif

#endif	/* !defined(HAS_TEST_AND_SET) */

/* Blow up if we didn't have any way to do spinlocks */
#ifndef HAS_TEST_AND_SET
#error PostgreSQL does not have native spinlock support on this platform.  To continue the compilation, rerun configure using --disable-spinlocks.  However, performance will be poor.  Please report this to pgsql-bugs@postgresql.org.
#endif

#else	/* !HAVE_SPINLOCKS */


typedef int slock_t;

extern bool s_lock_free_sema(volatile slock_t *lock);
extern void s_unlock_sema(volatile slock_t *lock);
extern void s_init_lock_sema(volatile slock_t *lock, bool nested);
extern int tas_sema(volatile slock_t *lock);

#define S_LOCK_FREE(lock)	s_lock_free_sema(lock)
#define S_UNLOCK(lock)	 s_unlock_sema(lock)
#define S_INIT_LOCK(lock)	s_init_lock_sema(lock, false)
#define TAS(lock)	tas_sema(lock)

#endif	/* HAVE_SPINLOCKS */

/*
 * Default Definitions - override these above as needed.
 */

#if !defined(S_LOCK)
#define S_LOCK(lock) \
	(TAS(lock) ? s_lock((lock), __FILE__, __LINE__, PG_FUNCNAME_MACRO) : 0)
#endif	 /* S_LOCK */

#if !defined(S_LOCK_FREE)
#define S_LOCK_FREE(lock)	(*(lock) == 0)
#endif	 /* S_LOCK_FREE */

#if !defined(S_UNLOCK)

#define USE_DEFAULT_S_UNLOCK
extern void s_unlock(volatile slock_t *lock);
#define S_UNLOCK(lock)		s_unlock(lock)
#endif	 /* S_UNLOCK */

#if !defined(S_INIT_LOCK)
#define S_INIT_LOCK(lock)	S_UNLOCK(lock)
#endif	 /* S_INIT_LOCK */

#if !defined(SPIN_DELAY)
#define SPIN_DELAY()	((void) 0)
#endif	 /* SPIN_DELAY */

#if !defined(TAS)
extern int tas(volatile slock_t *lock); /* in port/.../tas.s, or
 * s_lock.c */

#define TAS(lock)		tas(lock)
#endif	 /* TAS */

#if !defined(TAS_SPIN)
#define TAS_SPIN(lock)	TAS(lock)
#endif	 /* TAS_SPIN */

extern slock_t dummy_spinlock;

/*
 * Platform-independent out-of-line support routines
 */
extern int s_lock(volatile slock_t *lock, const char *file, int line,
		const char *func);

/* Support for dynamic adjustment of spins_per_delay */
#define DEFAULT_SPINS_PER_DELAY  100

extern void set_spins_per_delay(int shared_spins_per_delay);
extern int update_spins_per_delay(int shared_spins_per_delay);


typedef struct {
	int spins;
	int delays;
	int cur_delay;
	const char *file;
	int line;
	const char *func;
} SpinDelayStatus;

static inline void init_spin_delay(SpinDelayStatus *status, const char *file,
		int line, const char *func) {
	status->spins = 0;
	status->delays = 0;
	status->cur_delay = 0;
	status->file = file;
	status->line = line;
	status->func = func;
}

#define init_local_spin_delay(status) init_spin_delay(status, __FILE__, __LINE__, PG_FUNCNAME_MACRO)
void perform_spin_delay(SpinDelayStatus *status);
void finish_spin_delay(SpinDelayStatus *status);

#endif	 /* S_LOCK_H */

