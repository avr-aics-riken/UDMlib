###################################################################################
#
# UDMlib - Unstructured Data Management Library
#
# Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
# All rights reserved.
#
# Copyright (c) 2015 Advanced Institute for Computational Science, RIKEN.
# All rights reserved.
#
###################################################################################

AM_CPPFLAGS = -I$(top_srcdir)/include

AM_CFLAGS = @MPICH_FLAGS@ @CGNS_FLAGS@ @TP_FLAGS@  @ZOLTAN_FLAGS@  @UDM_FLAGS@
AM_LDFLAGS = @UDM_LDFLAGS@ @ZOLTAN_LDFLAGS@ @CGNS_LDFLAGS@ @MPICH_LDFLAGS@
LDADD = -L$(top_builddir)/src @UDM_LIBS@ @ZOLTAN_LIBS@ @CGNS_LIBS@ @MPICH_LIBS@ @TP_LDFLAGS@

noinst_PROGRAMS = partition \
                  partition_weight \
                  partition_solutions

partition_SOURCES = partition.c
partition_weight_SOURCES = partition_weight.c
partition_solutions_SOURCES = partition_solutions.c
