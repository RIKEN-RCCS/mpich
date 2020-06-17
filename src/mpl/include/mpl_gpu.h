/*
 *  Copyright (C) by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#ifndef MPL_GPU_H_INCLUDED
#define MPL_GPU_H_INCLUDED

#include "mplconfig.h"

#ifdef MPL_HAVE_CUDA
#include "mpl_gpu_cuda.h"
#elif defined MPL_HAVE_ZE
#include "mpl_gpu_ze.h"
#else
#include "mpl_gpu_fallback.h"
#endif

typedef enum {
    MPL_GPU_POINTER_UNREGISTERED_HOST = 0,
    MPL_GPU_POINTER_REGISTERED_HOST,
    MPL_GPU_POINTER_DEV,
    MPL_GPU_POINTER_MANAGED
} MPL_pointer_type_t;

typedef struct {
    MPL_pointer_type_t type;
    MPL_gpu_device_handle_t device;
} MPL_pointer_attr_t;

int MPL_gpu_query_pointer_attr(const void *ptr, MPL_pointer_attr_t * attr);

int MPL_gpu_ipc_handle_create(const void *ptr, MPL_gpu_ipc_mem_handle_t * ipc_handle);
int MPL_gpu_ipc_handle_map(MPL_gpu_ipc_mem_handle_t ipc_handle, MPL_gpu_device_handle_t dev_handle,
                           void **ptr);
int MPL_gpu_ipc_handle_unmap(void *ptr, MPL_gpu_ipc_mem_handle_t ipc_handle);

int MPL_gpu_malloc_host(void **ptr, size_t size);
int MPL_gpu_free_host(void *ptr);
int MPL_gpu_register_host(const void *ptr, size_t size);
int MPL_gpu_unregister_host(const void *ptr);

int MPL_gpu_malloc(void **ptr, size_t size, MPL_gpu_device_handle_t h_device);
int MPL_gpu_free(void *ptr);

int MPL_gpu_init(void);
int MPL_gpu_finalize(void);

int MPL_gpu_get_dev_id(MPL_gpu_device_handle_t dev_handle, int *dev_id);
int MPL_gpu_get_dev_handle(int dev_id, MPL_gpu_device_handle_t * dev_handle);

#endif /* ifndef MPL_GPU_H_INCLUDED */
