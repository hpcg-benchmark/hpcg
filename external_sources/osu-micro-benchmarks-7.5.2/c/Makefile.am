SUBDIRS =

if CUDA
    dist_pkglibexec_SCRIPTS = get_local_rank
endif

if ROCM
    dist_pkglibexec_SCRIPTS = get_local_rank
endif

if MPI
    SUBDIRS += mpi
endif

if OSHM
    SUBDIRS += openshmem
endif

if UPC
    SUBDIRS += upc
endif

if UPCXX
    SUBDIRS += upcxx
endif

if NCCL
    SUBDIRS += xccl
endif

if RCCL
    SUBDIRS += xccl
endif
