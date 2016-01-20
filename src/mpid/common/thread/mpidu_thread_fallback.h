/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#if !defined(MPIDU_THREAD_H_INCLUDED)
#define MPIDU_THREAD_H_INCLUDED

#include "mpiutil.h"

/* some important critical section names:
 *   GLOBAL - entered/exited at beginning/end of (nearly) every MPI_ function
 *   INIT - entered before MPID_Init and exited near the end of MPI_Init(_thread)
 * See the analysis of the MPI routines for thread usage properties.  Those
 * routines considered "Access Only" do not require GLOBAL.  That analysis
 * was very general; in MPICH, some routines may have internal shared
 * state that isn't required by the MPI specification.  Perhaps the
 * best example of this is the MPI_ERROR_STRING routine, where the
 * instance-specific error messages make use of shared state, and hence
 * must be accessed in a thread-safe fashion (e.g., require an GLOBAL
 * critical section).  With such routines removed, the set of routines
 * that (probably) do not require GLOBAL include:
 *
 * MPI_CART_COORDS, MPI_CART_GET, MPI_CART_MAP, MPI_CART_RANK, MPI_CART_SHIFT,
 * MPI_CART_SUB, MPI_CARTDIM_GET, MPI_COMM_GET_NAME,
 * MPI_COMM_RANK, MPI_COMM_REMOTE_SIZE,
 * MPI_COMM_SET_NAME, MPI_COMM_SIZE, MPI_COMM_TEST_INTER, MPI_ERROR_CLASS,
 * MPI_FILE_GET_AMODE, MPI_FILE_GET_ATOMICITY, MPI_FILE_GET_BYTE_OFFSET,
 * MPI_FILE_GET_POSITION, MPI_FILE_GET_POSITION_SHARED, MPI_FILE_GET_SIZE
 * MPI_FILE_GET_TYPE_EXTENT, MPI_FILE_SET_SIZE,
g * MPI_FINALIZED, MPI_GET_COUNT, MPI_GET_ELEMENTS, MPI_GRAPH_GET,
 * MPI_GRAPH_MAP, MPI_GRAPH_NEIGHBORS, MPI_GRAPH_NEIGHBORS_COUNT,
 * MPI_GRAPHDIMS_GET, MPI_GROUP_COMPARE, MPI_GROUP_RANK,
 * MPI_GROUP_SIZE, MPI_GROUP_TRANSLATE_RANKS, MPI_INITIALIZED,
 * MPI_PACK, MPI_PACK_EXTERNAL, MPI_PACK_SIZE, MPI_TEST_CANCELLED,
 * MPI_TOPO_TEST, MPI_TYPE_EXTENT, MPI_TYPE_GET_ENVELOPE,
 * MPI_TYPE_GET_EXTENT, MPI_TYPE_GET_NAME, MPI_TYPE_GET_TRUE_EXTENT,
 * MPI_TYPE_LB, MPI_TYPE_SET_NAME, MPI_TYPE_SIZE, MPI_TYPE_UB, MPI_UNPACK,
 * MPI_UNPACK_EXTERNAL, MPI_WIN_GET_NAME, MPI_WIN_SET_NAME
 *
 * Some of the routines that could be read-only, but internally may
 * require access or updates to shared data include
 * MPI_COMM_COMPARE (creation of group sets)
 * MPI_COMM_SET_ERRHANDLER (reference count on errhandler)
 * MPI_COMM_CALL_ERRHANDLER (actually ok, but risk high, usage low)
 * MPI_FILE_CALL_ERRHANDLER (ditto)
 * MPI_WIN_CALL_ERRHANDLER (ditto)
 * MPI_ERROR_STRING (access to instance-specific string, which could
 *                   be overwritten by another thread)
 * MPI_FILE_SET_VIEW (setting view a big deal)
 * MPI_TYPE_COMMIT (could update description of type internally,
 *                  including creating a new representation.  Should
 *                  be ok, but, like call_errhandler, low usage)
 *
 * Note that other issues may force a routine to include the GLOBAL
 * critical section, such as debugging information that requires shared
 * state.  Such situations should be avoided where possible.
 */

typedef MPIU_Thread_mutex_t MPIDU_Thread_mutex_t;
typedef MPIU_Thread_cond_t  MPIDU_Thread_cond_t;
typedef MPIU_Thread_id_t    MPIDU_Thread_id_t;
typedef MPIU_Thread_tls_t   MPIDU_Thread_tls_t;
typedef MPIU_Thread_func_t  MPIDU_Thread_func_t;

/*M MPIDU_THREAD_CS_ENTER - Enter a named critical section

  Input Parameters:
+ _name - name of the critical section
- _context - A context (typically an object) of the critical section

M*/
#define MPIDU_THREAD_CS_ENTER(name, mutex) MPIDUI_THREAD_CS_ENTER_##name(mutex)

#if defined(MPICH_IS_THREADED)

#if MPICH_THREAD_GRANULARITY == MPICH_THREAD_GRANULARITY_GLOBAL

#define MPIDUI_THREAD_CS_ENTER_GLOBAL(mutex)                            \
    do {                                                                \
        if (MPIR_ThreadInfo.isThreaded) {                               \
            int err_ = 0;                                               \
            MPIU_DBG_MSG(MPIR_DBG_THREAD, TYPICAL, "recursive locking GLOBAL mutex"); \
            MPIU_THREAD_CS_ENTER_RECURSIVE(mutex, &err_);               \
            MPIU_Assert(err_ == 0);                                     \
        }                                                               \
    } while (0)
#define MPIDUI_THREAD_CS_ENTER_ALLGRAN(mutex)                           \
    do {                                                                \
        if (MPIR_ThreadInfo.isThreaded) {                               \
            int err_ = 0;                                               \
            MPIU_DBG_MSG(MPIR_DBG_THREAD, TYPICAL, "non-recursive locking ALLGRAN mutex"); \
            MPIU_THREAD_CS_ENTER_NONRECURSIVE(mutex, &err_);            \
            MPIU_Assert(err_ == 0);                                     \
        }                                                               \
    } while (0)
#define MPIDUI_THREAD_CS_ENTER_POBJ(mutex) do {} while (0)

#else /* MPICH_THREAD_GRANULARITY == MPICH_THREAD_GRANULARITY_POBJ */

#define MPIDUI_THREAD_CS_ENTER_POBJ(mutex)                              \
    do {                                                                \
        if (MPIR_ThreadInfo.isThreaded) {                               \
            int err_ = 0;                                               \
            MPIU_DBG_MSG(MPIR_DBG_THREAD, TYPICAL, "non-recursive locking POBJ mutex"); \
            MPIU_THREAD_CS_ENTER_NONRECURSIVE(mutex, &err_);            \
            MPIU_Assert(err_ == 0);                                     \
        }                                                               \
    } while (0)
#define MPIDUI_THREAD_CS_ENTER_ALLGRAN  MPIDUI_THREAD_CS_ENTER_POBJ
#define MPIDUI_THREAD_CS_ENTER_GLOBAL(mutex) do {} while (0)

#endif  /* MPICH_THREAD_GRANULARITY */

#else  /* !defined(MPICH_IS_THREADED) */

#define MPIDUI_THREAD_CS_ENTER_GLOBAL(mutex) do {} while (0)
#define MPIDUI_THREAD_CS_ENTER_ALLGRAN(mutex) do {} while (0)
#define MPIDUI_THREAD_CS_ENTER_POBJ(mutex) do {} while (0)

#endif /* MPICH_IS_THREADED */


/*M MPIDU_THREAD_CS_EXIT - Exit a named critical section

  Input Parameters:
+ _name - cname of the critical section
- _context - A context (typically an object) of the critical section

M*/
#define MPIDU_THREAD_CS_EXIT(name, mutex) MPIDUI_THREAD_CS_EXIT_##name(mutex)

#if defined(MPICH_IS_THREADED)

#if MPICH_THREAD_GRANULARITY == MPICH_THREAD_GRANULARITY_GLOBAL

#define MPIDUI_THREAD_CS_EXIT_GLOBAL(mutex)                             \
    do {                                                                \
        if (MPIR_ThreadInfo.isThreaded) {                               \
            int err_ = 0;                                               \
            MPIU_DBG_MSG(MPIR_DBG_THREAD, TYPICAL, "recursive unlocking GLOBAL mutex"); \
            MPIU_THREAD_CS_EXIT_RECURSIVE(mutex, &err_);                \
            MPIU_Assert(err_ == 0);                                     \
        }                                                               \
    } while (0)
#define MPIDUI_THREAD_CS_EXIT_ALLGRAN(mutex)                            \
    do {                                                                \
        if (MPIR_ThreadInfo.isThreaded) {                               \
            int err_ = 0;                                               \
            MPIU_DBG_MSG(MPIR_DBG_THREAD, TYPICAL, "non-recursive unlocking ALLGRAN mutex"); \
            MPIU_THREAD_CS_EXIT_NONRECURSIVE(mutex, &err_);             \
            MPIU_Assert(err_ == 0);                                     \
        }                                                               \
    } while (0)
#define MPIDUI_THREAD_CS_EXIT_POBJ(mutex) do {} while (0)

#else /* MPICH_THREAD_GRANULARITY == MPICH_THREAD_GRANULARITY_POBJ */

#define MPIDUI_THREAD_CS_EXIT_POBJ(mutex)                               \
    do {                                                                \
        if (MPIR_ThreadInfo.isThreaded) {                               \
            int err_ = 0;                                               \
            MPIU_DBG_MSG(MPIR_DBG_THREAD, TYPICAL, "non-recursive unlocking POBJ mutex"); \
            MPIU_THREAD_CS_EXIT_NONRECURSIVE(mutex, &err_);             \
            MPIU_Assert(err_ == 0);                                     \
        }                                                               \
    } while (0)
#define MPIDUI_THREAD_CS_EXIT_ALLGRAN  MPIDUI_THREAD_CS_EXIT_POBJ
#define MPIDUI_THREAD_CS_EXIT_GLOBAL(mutex) do {} while (0)

#endif  /* MPICH_THREAD_GRANULARITY */

#else  /* !defined(MPICH_IS_THREADED) */

#define MPIDUI_THREAD_CS_EXIT_GLOBAL(mutex) do {} while (0)
#define MPIDUI_THREAD_CS_EXIT_ALLGRAN(mutex) do {} while (0)
#define MPIDUI_THREAD_CS_EXIT_POBJ(mutex) do {} while (0)

#endif /* MPICH_IS_THREADED */


/*M MPIDU_THREAD_CS_YIELD - Temporarily release a critical section and yield
    to other threads

  Input Parameters:
+ _name - cname of the critical section
- _context - A context (typically an object) of the critical section

  M*/
#define MPIDU_THREAD_CS_YIELD(name, mutex) MPIDUI_THREAD_CS_YIELD_##name(mutex)

#if defined(MPICH_IS_THREADED)

#if MPICH_THREAD_GRANULARITY == MPICH_THREAD_GRANULARITY_GLOBAL

#define MPIDUI_THREAD_CS_YIELD_GLOBAL(mutex)                            \
    do {                                                                \
        if (MPIR_ThreadInfo.isThreaded) {                               \
            int err_ = 0;                                               \
            MPIU_DBG_MSG(MPIR_DBG_THREAD, TYPICAL, "recursive yielding GLOBAL mutex"); \
            MPIU_THREAD_CS_YIELD_RECURSIVE(mutex, &err_);               \
            MPIU_Assert(err_ == 0);                                     \
        }                                                               \
    } while (0)
#define MPIDUI_THREAD_CS_YIELD_ALLGRAN(mutex)                           \
    do {                                                                \
        if (MPIR_ThreadInfo.isThreaded) {                               \
            int err_ = 0;                                               \
            MPIU_DBG_MSG(MPIR_DBG_THREAD, TYPICAL, "non-recursive yielding ALLGRAN mutex"); \
            MPIU_THREAD_CS_YIELD_NONRECURSIVE(mutex, &err_);            \
            MPIU_Assert(err_ == 0);                                     \
        }                                                               \
    } while (0)
#define MPIDUI_THREAD_CS_YIELD_POBJ(mutex) do {} while (0)

#else /* MPICH_THREAD_GRANULARITY == MPICH_THREAD_GRANULARITY_POBJ */

#define MPIDUI_THREAD_CS_YIELD_POBJ(mutex)                              \
    do {                                                                \
        if (MPIR_ThreadInfo.isThreaded) {                               \
            int err_ = 0;                                               \
            MPIU_DBG_MSG(MPIR_DBG_THREAD, TYPICAL, "non-recursive yielding POBJ mutex"); \
            MPIU_THREAD_CS_YIELD_NONRECURSIVE(mutex, &err_);            \
            MPIU_Assert(err_ == 0);                                     \
        }                                                               \
    } while (0)
#define MPIDUI_THREAD_CS_YIELD_ALLGRAN  MPIDUI_THREAD_CS_YIELD_POBJ
#define MPIDUI_THREAD_CS_YIELD_GLOBAL(mutex) do {} while (0)

#endif  /* MPICH_THREAD_GRANULARITY */

#else  /* !defined(MPICH_IS_THREADED) */

#define MPIDUI_THREAD_CS_YIELD_GLOBAL(mutex) do {} while (0)
#define MPIDUI_THREAD_CS_YIELD_ALLGRAN(mutex) do {} while (0)
#define MPIDUI_THREAD_CS_YIELD_POBJ(mutex) do {} while (0)

#endif /* MPICH_IS_THREADED */




/*@
  MPIDU_Thread_create - create a new thread

  Input Parameters:
+ func - function to run in new thread
- data - data to be passed to thread function

  Output Parameters:
+ id - identifier for the new thread
- err - location to store the error code; pointer may be NULL; error is zero for success, non-zero if a failure occurred

  Notes:
  The thread is created in a detach state, meaning that is may not be waited upon.  If another thread needs to wait for this
  thread to complete, the threads must provide their own synchronization mechanism.
@*/
#define MPIDU_Thread_create(func_, data_, id_, err_ptr_)        \
    do {                                                        \
        MPIU_Thread_create(func_, data_, id_, err_ptr_);        \
        MPIU_Assert(*err_ptr_ == 0);                            \
    } while (0)

/*@
  MPIDU_Thread_exit - exit from the current thread
@*/
#define MPIDU_Thread_exit         MPIU_Thread_exit

/*@
  MPIDU_Thread_self - get the identifier of the current thread

  Output Parameter:
. id - identifier of current thread
@*/
#define MPIDU_Thread_self         MPIU_Thread_self

/*@
  MPIDU_Thread_same - compare two threads identifiers to see if refer to the same thread

  Input Parameters:
+ id1 - first identifier
- id2 - second identifier

  Output Parameter:
. same - TRUE if the two threads identifiers refer to the same thread; FALSE otherwise
@*/
#define MPIDU_Thread_same       MPIU_Thread_same

/*@
  MPIDU_Thread_yield - voluntarily relinquish the CPU, giving other threads an opportunity to run
@*/
#define MPIDU_Thread_yield(mutex_ptr_, err_ptr_)        \
    do {                                                \
        MPIU_Thread_yield(mutex_ptr_, err_ptr_);        \
        MPIU_Assert(*err_ptr_ == 0);                    \
    } while (0)

/*
 *    Mutexes
 */

/*@
  MPIDU_Thread_mutex_create - create a new mutex

  Output Parameters:
+ mutex - mutex
- err - error code (non-zero indicates an error has occurred)
@*/
#define MPIDU_Thread_mutex_create(mutex_ptr_, err_ptr_) \
    do {                                                \
        MPIU_Thread_mutex_create(mutex_ptr_, err_ptr_); \
        MPIU_Assert(*err_ptr_ == 0);                    \
    } while (0)

/*@
  MPIDU_Thread_mutex_destroy - destroy an existing mutex

  Input Parameter:
. mutex - mutex

  Output Parameter:
. err - location to store the error code; pointer may be NULL; error is zero for success, non-zero if a failure occurred
@*/
#define MPIDU_Thread_mutex_destroy(mutex_ptr_, err_ptr_)        \
    do {                                                        \
        MPIU_Thread_mutex_destroy(mutex_ptr_, err_ptr_);        \
        MPIU_Assert(*err_ptr_ == 0);                            \
    } while (0)

/*@
  MPIDU_Thread_lock - acquire a mutex

  Input Parameter:
. mutex - mutex
@*/
#define MPIDU_Thread_mutex_lock(mutex_ptr_, err_ptr_)   \
    do {                                                \
        MPIU_Thread_mutex_lock(mutex_ptr_, err_ptr_);   \
        MPIU_Assert(*err_ptr_ == 0);                    \
    } while (0)

/*@
  MPIDU_Thread_unlock - release a mutex

  Input Parameter:
. mutex - mutex
@*/
#define MPIDU_Thread_mutex_unlock(mutex_ptr_, err_ptr_) \
    do {                                                \
        MPIU_Thread_mutex_unlock(mutex_ptr_, err_ptr_); \
        MPIU_Assert(*err_ptr_ == 0);                    \
    } while (0)

/*
 * Condition Variables
 */

/*@
  MPIDU_Thread_cond_create - create a new condition variable

  Output Parameters:
+ cond - condition variable
- err - location to store the error code; pointer may be NULL; error is zero for success, non-zero if a failure occurred
@*/
#define MPIDU_Thread_cond_create(cond_ptr_, err_ptr_)   \
    do {                                                \
        MPIU_Thread_cond_create(cond_ptr_, err_ptr_);   \
        MPIU_Assert(*err_ptr_ == 0);                    \
    } while (0)

/*@
  MPIDU_Thread_cond_destroy - destroy an existinga condition variable

  Input Parameter:
. cond - condition variable

  Output Parameter:
. err - location to store the error code; pointer may be NULL; error is zero
        for success, non-zero if a failure occurred
@*/
#define MPIDU_Thread_cond_destroy(cond_ptr_, err_ptr_)  \
    do {                                                \
        MPIU_Thread_cond_destroy(cond_ptr_, err_ptr_);  \
        MPIU_Assert(*err_ptr_ == 0);                    \
    } while (0)

/*@
  MPIDU_Thread_cond_wait - wait (block) on a condition variable

  Input Parameters:
+ cond - condition variable
- mutex - mutex

  Notes:
  This function may return even though another thread has not requested that a
  thread be released.  Therefore, the calling
  program must wrap the function in a while loop that verifies program state
  has changed in a way that warrants letting the
  thread proceed.
@*/
#define MPIDU_Thread_cond_wait(cond_ptr_, mutex_ptr_, err_ptr_)         \
    do {                                                                \
        MPIU_Thread_cond_wait(cond_ptr_, mutex_ptr_, err_ptr_);         \
        MPIU_Assert_fmt_msg(*((int *) err_ptr_) == 0,                   \
                            ("cond_wait failed, err=%d (%s)", *((int *) err_ptr_), strerror(*((int *) err_ptr_)))); \
    } while (0)

/*@
  MPIDU_Thread_cond_broadcast - release all threads currently waiting on a condition variable

  Input Parameter:
. cond - condition variable
@*/
#define MPIDU_Thread_cond_broadcast(cond_ptr_, err_ptr_)                \
    do {                                                                \
        MPIU_Thread_cond_broadcast(cond_ptr_, err_ptr_);                \
        MPIU_Assert_fmt_msg(*((int *) err_ptr_) == 0,                   \
                            ("cond_broadcast failed, err=%d (%s)", *((int *) err_ptr_), strerror(*((int *) err_ptr_)))); \
    } while (0)

/*@
  MPIDU_Thread_cond_signal - release one thread currently waitng on a condition variable

  Input Parameter:
. cond - condition variable
@*/
#define MPIDU_Thread_cond_signal(cond_ptr_, err_ptr_)                   \
    do {                                                                \
        MPIU_Thread_cond_signal(cond_ptr_, err_ptr_);                   \
        MPIU_Assert_fmt_msg(*((int *) err_ptr_) == 0,                   \
                            ("cond_signal failed, err=%d (%s)", *((int *) err_ptr_), strerror(*((int *) err_ptr_)))); \
    } while (0)

/*
 * Thread Local Storage
 */
/*@
  MPIDU_Thread_tls_create - create a thread local storage space

  Input Parameter:
. exit_func - function to be called when the thread exists; may be NULL if a
  callback is not desired

  Output Parameters:
+ tls - new thread local storage space
- err - location to store the error code; pointer may be NULL; error is zero
        for success, non-zero if a failure occurred
@*/
#define MPIDU_Thread_tls_create(exit_func_ptr_, tls_ptr_, err_ptr_)     \
    do {                                                                \
        MPIU_Thread_tls_create(exit_func_ptr_, tls_ptr_, err_ptr_);     \
        MPIU_Assert(*(int *) err_ptr_ == 0);                            \
    } while (0)

/*@
  MPIDU_Thread_tls_destroy - destroy a thread local storage space

  Input Parameter:
. tls - thread local storage space to be destroyed

  Output Parameter:
. err - location to store the error code; pointer may be NULL; error is zero
        for success, non-zero if a failure occurred

  Notes:
  The destroy function associated with the thread local storage will not
  called after the space has been destroyed.
@*/
#define MPIDU_Thread_tls_destroy(tls_ptr_, err_ptr_)    \
    do {                                                \
        MPIU_Thread_tls_destroy(tls_ptr_, err_ptr_);    \
        MPIU_Assert(*(int *) err_ptr_ == 0);            \
    } while (0)

/*@
  MPIDU_Thread_tls_set - associate a value with the current thread in the
  thread local storage space

  Input Parameters:
+ tls - thread local storage space
- value - value to associate with current thread
@*/
#define MPIDU_Thread_tls_set(tls_ptr_, value_, err_ptr_)                \
    do {                                                                \
        MPIU_Thread_tls_set(tls_ptr_, value_, err_ptr_);                \
        MPIU_Assert_fmt_msg(*((int *) err_ptr_) == 0,                   \
                            ("tls_set failed, err=%d (%s)", *((int *) err_ptr_), strerror(*((int *) err_ptr_)))); \
    } while (0)

/*@
  MPIDU_Thread_tls_get - obtain the value associated with the current thread
  from the thread local storage space

  Input Parameter:
. tls - thread local storage space

  Output Parameter:
. value - value associated with current thread
@*/
#define MPIDU_Thread_tls_get(tls_ptr_, value_ptr_, err_ptr_)            \
    do {                                                                \
        MPIU_Thread_tls_get(tls_ptr_, value_ptr_, err_ptr_);            \
        MPIU_Assert_fmt_msg(*((int *) err_ptr_) == 0,                   \
                            ("tls_get failed, err=%d (%s)", *((int *) err_ptr_), strerror(*((int *) err_ptr_)))); \
    } while (0)


#define MPIDU_THREADPRIV_INITKEY                                        \
    do {                                                                \
        int err_ = 0;                                                   \
        MPIU_THREADPRIV_INITKEY(MPIR_ThreadInfo.isThreaded, &err_);     \
        MPIU_Assert(err_ == 0);                                         \
    } while (0)

#define MPIDU_THREADPRIV_INIT                                   \
    do {                                                        \
        int err_ = 0;                                           \
        MPIU_THREADPRIV_INIT(MPIR_ThreadInfo.isThreaded, &err_);        \
        MPIU_Assert(err_ == 0);                                 \
    } while (0)

#define MPIDU_THREADPRIV_GET                                    \
    do {                                                        \
        int err_ = 0;                                           \
        MPIU_THREADPRIV_GET(MPIR_ThreadInfo.isThreaded, &err_);  \
        MPIU_Assert(err_ == 0);                                 \
    } while (0)

#define MPIDU_THREADPRIV_DECL     MPIU_THREADPRIV_DECL
#define MPIDU_THREADPRIV_FIELD    MPIU_THREADPRIV_FIELD

#define MPIDU_THREADPRIV_FINALIZE                                       \
    do {                                                                \
        int err_ = 0;                                                   \
        MPIU_THREADPRIV_FINALIZE(MPIR_ThreadInfo.isThreaded, &err_);    \
        MPIU_Assert(err_ == 0);                                         \
    } while (0)


#endif /* !defined(MPIDU_THREAD_H_INCLUDED) */
