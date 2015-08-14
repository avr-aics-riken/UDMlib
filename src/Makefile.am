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

AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -I$(top_srcdir)/include

lib_LIBRARIES = libudm.a

libudm_a_CXXFLAGS = @UDM_SYMBOLS@ @MPICH_FLAGS@ @CGNS_FLAGS@ @TP_FLAGS@  @ZOLTAN_FLAGS@

libudm_a_SOURCES = \
		./UdmBase.cpp \
		./UdmErrorHandler.cpp \
		./udmlib_cc.cpp \
		./config/UdmFilePathConfig.cpp \
		./config/UdmUnitListConfig.cpp \
		./config/UdmFlowSolutionListConfig.cpp \
		./config/UdmConfigBase.cpp \
		./config/UdmTimeSliceConfig.cpp \
		./config/UdmDfiValue.cpp \
		./config/UdmMpiConfig.cpp \
		./config/UdmSettingsConfig.cpp \
		./config/UdmProcessConfig.cpp \
		./config/UdmSolutionFieldConfig.cpp \
		./config/UdmDfiConfig.cpp \
		./config/UdmDomainConfig.cpp \
		./config/UdmFileInfoConfig.cpp \
		./model/UdmEntityVoxels.cpp \
		./model/UdmSections.cpp \
		./model/UdmSolid.cpp \
		./model/UdmGlobalRankid.cpp \
		./model/UdmBar.cpp \
		./model/UdmCell.cpp \
		./model/UdmComponent.cpp \
		./model/UdmFlowSolutions.cpp \
		./model/UdmIterativeData.cpp \
		./model/UdmGeneral.cpp \
		./model/UdmElements.cpp \
		./model/UdmUserDefinedDatas.cpp \
		./model/UdmSolutionData.cpp \
		./model/UdmZone.cpp \
		./model/UdmGridCoordinates.cpp \
		./model/UdmModel.cpp \
		./model/UdmShell.cpp \
		./model/UdmCoordsValue.cpp \
		./model/UdmNode.cpp \
		./model/UdmEntity.cpp \
		./model/UdmRankConnectivity.cpp \
		./model/UdmInfo.cpp \
		./partition/UdmLoadBalance.cpp \
		./utils/UdmSearchTable.cpp \
		./utils/UdmSerialization.cpp \
		./utils/UdmStopWatch.cpp \
		./utils/UdmScannerCells.cpp 

udm_includedir = $(includedir)
nobase_include_HEADERS = \
		$(top_srcdir)/include/udm_errorno.h \
		$(top_srcdir)/include/udm_version.h \
		$(top_srcdir)/include/udm_memutils.h \
		$(top_srcdir)/include/udm_mpiutils.h \
		$(top_srcdir)/include/udmlib.h \
		$(top_srcdir)/include/udm_pathutils.h \
		$(top_srcdir)/include/udm_define.h \
		$(top_srcdir)/include/udm_strutils.h \
		$(top_srcdir)/include/UdmErrorHandler.h \
		$(top_srcdir)/include/config/UdmFlowSolutionListConfig.h \
		$(top_srcdir)/include/config/UdmSettingsConfig.h \
		$(top_srcdir)/include/config/UdmDfiKeywords.h \
		$(top_srcdir)/include/config/UdmMpiConfig.h \
		$(top_srcdir)/include/config/UdmDfiConfig.h \
		$(top_srcdir)/include/config/UdmUnitListConfig.h \
		$(top_srcdir)/include/config/UdmDomainConfig.h \
		$(top_srcdir)/include/config/UdmDfiValue.h \
		$(top_srcdir)/include/config/UdmFilePathConfig.h \
		$(top_srcdir)/include/config/UdmFileInfoConfig.h \
		$(top_srcdir)/include/config/UdmSolutionFieldConfig.h \
		$(top_srcdir)/include/config/UdmTimeSliceConfig.h \
		$(top_srcdir)/include/config/UdmProcessConfig.h \
		$(top_srcdir)/include/config/UdmConfigBase.h \
		$(top_srcdir)/include/UdmBase.h \
		$(top_srcdir)/include/model/UdmFlowSolutions.h \
		$(top_srcdir)/include/model/UdmShell.h \
		$(top_srcdir)/include/model/UdmUserDefinedDatas.h \
		$(top_srcdir)/include/model/UdmEntity.h \
		$(top_srcdir)/include/model/UdmRankConnectivity.h \
		$(top_srcdir)/include/model/UdmCell.h \
		$(top_srcdir)/include/model/UdmNode.h \
		$(top_srcdir)/include/model/UdmSections.h \
		$(top_srcdir)/include/model/UdmGridCoordinates.h \
		$(top_srcdir)/include/model/UdmInfo.h \
		$(top_srcdir)/include/model/UdmIterativeData.h \
		$(top_srcdir)/include/model/UdmElements.h \
		$(top_srcdir)/include/model/UdmSolutionData.h \
		$(top_srcdir)/include/model/UdmGlobalRankid.h \
		$(top_srcdir)/include/model/UdmGeneral.h \
		$(top_srcdir)/include/model/UdmEntityVoxels.h \
		$(top_srcdir)/include/model/UdmSolid.h \
		$(top_srcdir)/include/model/UdmBar.h \
		$(top_srcdir)/include/model/UdmZone.h \
		$(top_srcdir)/include/model/UdmCoordsValue.h \
		$(top_srcdir)/include/model/UdmModel.h \
		$(top_srcdir)/include/model/UdmICellComponent.h \
		$(top_srcdir)/include/model/UdmComponent.h \
		$(top_srcdir)/include/partition/UdmLoadBalance.h \
		$(top_srcdir)/include/utils/UdmStopWatch.h \
		$(top_srcdir)/include/utils/UdmScannerCells.h \
		$(top_srcdir)/include/utils/UdmSerialization.h \
		$(top_srcdir)/include/utils/UdmSearchTable.h

all-local:
	mkdir -p $(top_builddir)/lib
	cp $(lib_LIBRARIES) $(top_builddir)/lib/

clean-local:
	rm -f $(top_builddir)/lib/$(lib_LIBRARIES)
	rm -rf $(top_builddir)/lib
