/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

/* Header protection (i.e., INEIGHBOR_ALLTOALL_TSP_LINEAR_ALGOS_PROTOTYPES_H_INCLUDED) is
 * intentionally omitted since this header might get included multiple
 * times within the same .c file. */

#include "tsp_namespace_def.h"

#undef MPIR_TSP_Ineighbor_alltoallv_allcomm_linear
#define MPIR_TSP_Ineighbor_alltoallv_allcomm_linear        MPIR_TSP_NAMESPACE(Ineighbor_alltoallv_allcomm_linear)
#undef MPIR_TSP_Ineighor_alltoallv_sched_allcomm_linear
#define MPIR_TSP_Ineighbor_alltoallv_sched_allcomm_linear  MPIR_TSP_NAMESPACE(Ineighbor_alltoallv_sched_allcomm_linear)

int MPIR_TSP_Ineighbor_alltoallv_sched_allcomm_linear(const void *sendbuf,
                                                      const MPI_Aint sendcounts[],
                                                      const MPI_Aint sdispls[],
                                                      MPI_Datatype sendtype, void *recvbuf,
                                                      const MPI_Aint recvcounts[],
                                                      const MPI_Aint rdispls[],
                                                      MPI_Datatype recvtype, MPIR_Comm * comm_ptr,
                                                      MPIR_TSP_sched_t * sched);
int MPIR_TSP_Ineighbor_alltoallv_allcomm_linear(const void *sendbuf, const MPI_Aint sendcounts[],
                                                const MPI_Aint sdispls[], MPI_Datatype sendtype,
                                                void *recvbuf, const MPI_Aint recvcounts[],
                                                const MPI_Aint rdispls[], MPI_Datatype recvtype,
                                                MPIR_Comm * comm_ptr, MPIR_Request ** req);
