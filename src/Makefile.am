#
# UDMlib - Unstructured Data Management Library
# Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
# All rights reserved.
#

AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -I$(top_builddir)/include

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
		$(top_builddir)/include/udm_errorno.h \
		$(top_builddir)/include/udm_version.h \
		$(top_builddir)/include/udm_memutils.h \
		$(top_builddir)/include/udm_mpiutils.h \
		$(top_builddir)/include/udmlib.h \
		$(top_builddir)/include/udm_pathutils.h \
		$(top_builddir)/include/udm_define.h \
		$(top_builddir)/include/udm_strutils.h \
		$(top_builddir)/include/UdmErrorHandler.h \
		$(top_builddir)/include/config/UdmFlowSolutionListConfig.h \
		$(top_builddir)/include/config/UdmSettingsConfig.h \
		$(top_builddir)/include/config/UdmDfiKeywords.h \
		$(top_builddir)/include/config/UdmMpiConfig.h \
		$(top_builddir)/include/config/UdmDfiConfig.h \
		$(top_builddir)/include/config/UdmUnitListConfig.h \
		$(top_builddir)/include/config/UdmDomainConfig.h \
		$(top_builddir)/include/config/UdmDfiValue.h \
		$(top_builddir)/include/config/UdmFilePathConfig.h \
		$(top_builddir)/include/config/UdmFileInfoConfig.h \
		$(top_builddir)/include/config/UdmSolutionFieldConfig.h \
		$(top_builddir)/include/config/UdmTimeSliceConfig.h \
		$(top_builddir)/include/config/UdmProcessConfig.h \
		$(top_builddir)/include/config/UdmConfigBase.h \
		$(top_builddir)/include/UdmBase.h \
		$(top_builddir)/include/model/UdmFlowSolutions.h \
		$(top_builddir)/include/model/UdmShell.h \
		$(top_builddir)/include/model/UdmUserDefinedDatas.h \
		$(top_builddir)/include/model/UdmEntity.h \
		$(top_builddir)/include/model/UdmRankConnectivity.h \
		$(top_builddir)/include/model/UdmCell.h \
		$(top_builddir)/include/model/UdmNode.h \
		$(top_builddir)/include/model/UdmSections.h \
		$(top_builddir)/include/model/UdmGridCoordinates.h \
		$(top_builddir)/include/model/UdmInfo.h \
		$(top_builddir)/include/model/UdmIterativeData.h \
		$(top_builddir)/include/model/UdmElements.h \
		$(top_builddir)/include/model/UdmSolutionData.h \
		$(top_builddir)/include/model/UdmGlobalRankid.h \
		$(top_builddir)/include/model/UdmGeneral.h \
		$(top_builddir)/include/model/UdmEntityVoxels.h \
		$(top_builddir)/include/model/UdmSolid.h \
		$(top_builddir)/include/model/UdmBar.h \
		$(top_builddir)/include/model/UdmZone.h \
		$(top_builddir)/include/model/UdmCoordsValue.h \
		$(top_builddir)/include/model/UdmModel.h \
		$(top_builddir)/include/model/UdmICellComponent.h \
		$(top_builddir)/include/model/UdmComponent.h \
		$(top_builddir)/include/partition/UdmLoadBalance.h \
		$(top_builddir)/include/utils/UdmStopWatch.h \
		$(top_builddir)/include/utils/UdmScannerCells.h \
		$(top_builddir)/include/utils/UdmSerialization.h \
		$(top_builddir)/include/utils/UdmSearchTable.h

all-local:
	mkdir -p $(top_builddir)/lib
	cp $(lib_LIBRARIES) $(top_builddir)/lib/

clean-local:
	rm -f $(top_builddir)/lib/$(lib_LIBRARIES)
	rm -rf $(top_builddir)/lib
