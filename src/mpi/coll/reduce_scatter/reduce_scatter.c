/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "mpiimpl.h"

/*
=== BEGIN_MPI_T_CVAR_INFO_BLOCK ===

cvars:
    - name        : MPIR_CVAR_REDUCE_SCATTER_COMMUTATIVE_LONG_MSG_SIZE
      category    : COLLECTIVE
      type        : int
      default     : 524288
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        the long message algorithm will be used if the operation is commutative
        and the send buffer size is >= this value (in bytes)

    - name        : MPIR_CVAR_REDUCE_SCATTER_INTRA_ALGORITHM
      category    : COLLECTIVE
      type        : enum
      default     : auto
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        Variable to select reduce_scatter algorithm
        auto - Internal algorithm selection (can be overridden with MPIR_CVAR_COLL_SELECTION_TUNING_JSON_FILE)
        nb                 - Force nonblocking algorithm
        noncommutative     - Force noncommutative algorithm
        pairwise           - Force pairwise algorithm
        recursive_doubling - Force recursive doubling algorithm
        recursive_halving  - Force recursive halving algorithm

    - name        : MPIR_CVAR_REDUCE_SCATTER_INTER_ALGORITHM
      category    : COLLECTIVE
      type        : enum
      default     : auto
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : |-
        Variable to select reduce_scatter algorithm
        auto - Internal algorithm selection (can be overridden with MPIR_CVAR_COLL_SELECTION_TUNING_JSON_FILE)
        nb                          - Force nonblocking algorithm
        remote_reduce_local_scatter - Force remote-reduce-local-scatter algorithm

    - name        : MPIR_CVAR_REDUCE_SCATTER_DEVICE_COLLECTIVE
      category    : COLLECTIVE
      type        : boolean
      default     : true
      class       : none
      verbosity   : MPI_T_VERBOSITY_USER_BASIC
      scope       : MPI_T_SCOPE_ALL_EQ
      description : >-
        This CVAR is only used when MPIR_CVAR_DEVICE_COLLECTIVES
        is set to "percoll".  If set to true, MPI_Reduce_scatter will
        allow the device to override the MPIR-level collective
        algorithms.  The device might still call the MPIR-level
        algorithms manually.  If set to false, the device-override
        will be disabled.

=== END_MPI_T_CVAR_INFO_BLOCK ===
*/

/* This is the machine-independent implementation of reduce_scatter. The algorithm is:

   Algorithm: MPI_Reduce_scatter

   If the operation is not commutative, we do the following:

   Possible improvements:

   End Algorithm: MPI_Reduce_scatter
*/


int MPIR_Reduce_scatter_allcomm_auto(const void *sendbuf, void *recvbuf,
                                     const MPI_Aint * recvcounts, MPI_Datatype datatype, MPI_Op op,
                                     MPIR_Comm * comm_ptr, MPIR_Errflag_t * errflag)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_Csel_coll_sig_s coll_sig = {
        .coll_type = MPIR_CSEL_COLL_TYPE__REDUCE_SCATTER,
        .comm_ptr = comm_ptr,

        .u.reduce_scatter.sendbuf = sendbuf,
        .u.reduce_scatter.recvbuf = recvbuf,
        .u.reduce_scatter.recvcounts = recvcounts,
        .u.reduce_scatter.datatype = datatype,
        .u.reduce_scatter.op = op,
    };

    MPII_Csel_container_s *cnt = MPIR_Csel_search(comm_ptr->csel_comm, coll_sig);
    MPIR_Assert(cnt);

    switch (cnt->id) {
        case MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIR_Reduce_scatter_intra_noncommutative:
            mpi_errno =
                MPIR_Reduce_scatter_intra_noncommutative(sendbuf, recvbuf, recvcounts, datatype, op,
                                                         comm_ptr, errflag);
            break;

        case MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIR_Reduce_scatter_intra_pairwise:
            mpi_errno =
                MPIR_Reduce_scatter_intra_pairwise(sendbuf, recvbuf, recvcounts, datatype, op,
                                                   comm_ptr, errflag);
            break;

        case MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIR_Reduce_scatter_intra_recursive_doubling:
            mpi_errno =
                MPIR_Reduce_scatter_intra_recursive_doubling(sendbuf, recvbuf, recvcounts, datatype,
                                                             op, comm_ptr, errflag);
            break;

        case MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIR_Reduce_scatter_intra_recursive_halving:
            mpi_errno =
                MPIR_Reduce_scatter_intra_recursive_halving(sendbuf, recvbuf, recvcounts, datatype,
                                                            op, comm_ptr, errflag);
            break;

        case MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIR_Reduce_scatter_inter_remote_reduce_local_scatter:
            mpi_errno =
                MPIR_Reduce_scatter_inter_remote_reduce_local_scatter(sendbuf, recvbuf, recvcounts,
                                                                      datatype, op, comm_ptr,
                                                                      errflag);
            break;

        case MPII_CSEL_CONTAINER_TYPE__ALGORITHM__MPIR_Reduce_scatter_allcomm_nb:
            mpi_errno =
                MPIR_Reduce_scatter_allcomm_nb(sendbuf, recvbuf, recvcounts, datatype, op, comm_ptr,
                                               errflag);
            break;

        default:
            MPIR_Assert(0);
    }

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

int MPIR_Reduce_scatter_impl(const void *sendbuf, void *recvbuf,
                             const MPI_Aint recvcounts[], MPI_Datatype datatype,
                             MPI_Op op, MPIR_Comm * comm_ptr, MPIR_Errflag_t * errflag)
{
    int mpi_errno = MPI_SUCCESS;

    if (comm_ptr->comm_kind == MPIR_COMM_KIND__INTRACOMM) {
        /* intracommunicator */
        switch (MPIR_CVAR_REDUCE_SCATTER_INTRA_ALGORITHM) {
            case MPIR_CVAR_REDUCE_SCATTER_INTRA_ALGORITHM_noncommutative:
                mpi_errno = MPIR_Reduce_scatter_intra_noncommutative(sendbuf, recvbuf,
                                                                     recvcounts, datatype, op,
                                                                     comm_ptr, errflag);
                break;
            case MPIR_CVAR_REDUCE_SCATTER_INTRA_ALGORITHM_pairwise:
                mpi_errno = MPIR_Reduce_scatter_intra_pairwise(sendbuf, recvbuf,
                                                               recvcounts, datatype, op, comm_ptr,
                                                               errflag);
                break;
            case MPIR_CVAR_REDUCE_SCATTER_INTRA_ALGORITHM_recursive_halving:
                mpi_errno = MPIR_Reduce_scatter_intra_recursive_halving(sendbuf, recvbuf,
                                                                        recvcounts, datatype, op,
                                                                        comm_ptr, errflag);
                break;
            case MPIR_CVAR_REDUCE_SCATTER_INTRA_ALGORITHM_recursive_doubling:
                mpi_errno = MPIR_Reduce_scatter_intra_recursive_doubling(sendbuf, recvbuf,
                                                                         recvcounts, datatype, op,
                                                                         comm_ptr, errflag);
                break;
            case MPIR_CVAR_REDUCE_SCATTER_INTRA_ALGORITHM_nb:
                mpi_errno = MPIR_Reduce_scatter_allcomm_nb(sendbuf, recvbuf,
                                                           recvcounts, datatype, op, comm_ptr,
                                                           errflag);
                break;
            case MPIR_CVAR_REDUCE_SCATTER_INTRA_ALGORITHM_auto:
                mpi_errno = MPIR_Reduce_scatter_allcomm_auto(sendbuf, recvbuf,
                                                             recvcounts, datatype, op, comm_ptr,
                                                             errflag);
                break;
            default:
                MPIR_Assert(0);
        }
    } else {
        /* intercommunicator */
        switch (MPIR_CVAR_REDUCE_SCATTER_INTER_ALGORITHM) {
            case MPIR_CVAR_REDUCE_SCATTER_INTER_ALGORITHM_remote_reduce_local_scatter:
                mpi_errno =
                    MPIR_Reduce_scatter_inter_remote_reduce_local_scatter(sendbuf, recvbuf,
                                                                          recvcounts, datatype, op,
                                                                          comm_ptr, errflag);
                break;
            case MPIR_CVAR_REDUCE_SCATTER_INTER_ALGORITHM_nb:
                mpi_errno = MPIR_Reduce_scatter_allcomm_nb(sendbuf, recvbuf,
                                                           recvcounts, datatype, op, comm_ptr,
                                                           errflag);
                break;
            case MPIR_CVAR_REDUCE_SCATTER_INTER_ALGORITHM_auto:
                mpi_errno = MPIR_Reduce_scatter_allcomm_auto(sendbuf, recvbuf, recvcounts,
                                                             datatype, op, comm_ptr, errflag);
                break;
            default:
                MPIR_Assert(0);
        }
    }
    MPIR_ERR_CHECK(mpi_errno);

  fn_exit:
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

int MPIR_Reduce_scatter(const void *sendbuf, void *recvbuf,
                        const MPI_Aint recvcounts[], MPI_Datatype datatype,
                        MPI_Op op, MPIR_Comm * comm_ptr, MPIR_Errflag_t * errflag)
{
    int mpi_errno = MPI_SUCCESS;
    void *in_recvbuf = recvbuf;
    void *host_sendbuf;
    void *host_recvbuf;
    int count = 0;

    for (int i = 0; i < MPIR_Comm_size(comm_ptr); i++)
        count += recvcounts[i];

    MPIR_Coll_host_buffer_alloc(sendbuf, recvbuf, count, datatype, &host_sendbuf, &host_recvbuf);
    if (host_sendbuf)
        sendbuf = host_sendbuf;
    if (host_recvbuf)
        recvbuf = host_recvbuf;

    if ((MPIR_CVAR_DEVICE_COLLECTIVES == MPIR_CVAR_DEVICE_COLLECTIVES_all) ||
        ((MPIR_CVAR_DEVICE_COLLECTIVES == MPIR_CVAR_DEVICE_COLLECTIVES_percoll) &&
         MPIR_CVAR_REDUCE_SCATTER_DEVICE_COLLECTIVE)) {
        mpi_errno =
            MPID_Reduce_scatter(sendbuf, recvbuf, recvcounts, datatype, op, comm_ptr, errflag);
    } else {
        mpi_errno = MPIR_Reduce_scatter_impl(sendbuf, recvbuf, recvcounts, datatype, op, comm_ptr,
                                             errflag);
    }

    /* Copy out data from host recv buffer to GPU buffer */
    if (host_recvbuf) {
        recvbuf = in_recvbuf;
        MPIR_Localcopy(host_recvbuf, count, datatype, recvbuf, count, datatype);
    }

    MPIR_Coll_host_buffer_free(host_sendbuf, host_recvbuf);

    return mpi_errno;
}
