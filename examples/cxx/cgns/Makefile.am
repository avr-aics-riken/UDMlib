#
# UDMlib - Unstructured Data Management Library
# Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
# All rights reserved.
#

AM_CPPFLAGS = -I$(top_builddir)/include

AM_CXXFLAGS = @MPICH_FLAGS@ @CGNS_FLAGS@ @TP_FLAGS@  @ZOLTAN_FLAGS@  @UDM_FLAGS@
AM_LDFLAGS = @UDM_LDFLAGS@ @ZOLTAN_LDFLAGS@ @CGNS_LDFLAGS@ @MPICH_LDFLAGS@
LDADD = @UDM_LIBS@ @ZOLTAN_LIBS@ @CGNS_LIBS@ @MPICH_LIBS@ @TP_LDFLAGS@

noinst_PROGRAMS = create_cgns \
                  create_multi \
                  read_cgns \
                  vtk2cgns \
                  user_defined

create_cgns_SOURCES = create_cgns.cpp
create_multi_SOURCES = create_multi.cpp
read_cgns_SOURCES = read_cgns.cpp
vtk2cgns_SOURCES = vtk2cgns.cpp
user_defined_SOURCES = user_defined.cpp

