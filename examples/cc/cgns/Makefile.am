#
# UDMlib - Unstructured Data Management Library
# Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
# All rights reserved.
#

AM_CPPFLAGS = -I$(top_builddir)/include

AM_CFLAGS = @MPICH_FLAGS@ @CGNS_FLAGS@ @TP_FLAGS@ @ZOLTAN_FLAGS@ @UDM_FLAGS@
AM_LDFLAGS = @UDM_LDFLAGS@ @ZOLTAN_LDFLAGS@ @CGNS_LDFLAGS@ @MPICH_LDFLAGS@
LDADD = @UDM_LIBS@ @ZOLTAN_LIBS@ @CGNS_LIBS@ @MPICH_LIBS@ @TP_LDFLAGS@

noinst_PROGRAMS = create_cgns \
                  read_cgns \
                  create_multi \
                  vtk2cgns \
                  user_defined

create_cgns_SOURCES = create_cgns.c
read_cgns_SOURCES = read_cgns.c
create_multi_SOURCES = create_multi.c
vtk2cgns_SOURCES = vtk2cgns.c
user_defined_SOURCES = user_defined.c

