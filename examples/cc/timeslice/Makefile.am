#
# UDMlib - Unstructured Data Management Library
# Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
# All rights reserved.
#

AM_CPPFLAGS = -I$(top_builddir)/include

AM_CFLAGS = @MPICH_FLAGS@ @CGNS_FLAGS@ @TP_FLAGS@  @ZOLTAN_FLAGS@  @UDM_FLAGS@
AM_LDFLAGS = @UDM_LDFLAGS@ @ZOLTAN_LDFLAGS@ @CGNS_LDFLAGS@ @MPICH_LDFLAGS@
LDADD = @UDM_LIBS@ @ZOLTAN_LIBS@ @CGNS_LIBS@ @MPICH_LIBS@ @TP_LDFLAGS@

noinst_PROGRAMS = timeslice \
                  create_timeslice

timeslice_SOURCES = timeslice.c
create_timeslice_SOURCES = create_timeslice.c

	