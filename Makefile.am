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

SUBDIRS = @MAKE_SUB_DIRS@

bin_SCRIPTS=udm-config
CLEANFILES = $(bin_SCRIPTS)
EXTRA_DIST = udm-config.in \
             doc/doxygen/doxygen.sh  doc/doxygen/udmlib.doxyfile \
             examples/data  \
             examples/cc/run_all.sh \
             examples/cc/dfi/dfi_run.sh  \
             examples/cc/cgns/cgns_run.sh \
             examples/cc/partition/partition_run.sh \
             examples/cc/timeslice/timeslice_run.sh \
             examples/cxx/run_all.sh \
             examples/cxx/dfi/dfi_run.sh \
             examples/cxx/cgns/cgns_run.sh \
             examples/cxx/partition/partition_run.sh \
             examples/cxx/timeslice/timeslice_run.sh \
             BUILD_DIR/.gitkeep
dist_data_DATA = AUTHORS COPYING ChangeLog README LICENSE NEWS README.md

all-local:
	chmod +x $(bin_SCRIPTS)
	mkdir -p $(top_builddir)/bin
	cp $(bin_SCRIPTS) $(top_builddir)/bin/
	
clean-local:
	find ./ -name *.obj | xargs rm -f
	find ./ -name *.exe | xargs rm -f
	find ./ -name *.lib | xargs rm -f

distclean-local: clean-local
	rm -f ./bin/*
	rm -f ./lib/*

dist-hook:
	find  $(distdir) -name Debug | xargs rm -rf
	find  $(distdir) -name Release | xargs rm -rf
	find  $(distdir) -regex ".+/\.[a-z]+"  | xargs rm -rf

