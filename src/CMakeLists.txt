###################################################################################
#
# UDMlib - Unstructured Data Management library
#
# Copyright (c) 2012-2015 Institute of Industrial Science, The University of Tokyo.
# All rights reserved.
#
# Copyright (c) 2015-2017 Advanced Institute for Computational Science, RIKEN.
# All rights reserved.
#
# Copyright (c) 2017 Research Institute for Information Technology (RIIT), Kyushu University.
# All rights reserved.
#
###################################################################################

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/config
    ${PROJECT_SOURCE_DIR}/include/model
    ${PROJECT_SOURCE_DIR}/include/partition
    ${PROJECT_SOURCE_DIR}/include/utils
    ${PROJECT_BINARY_DIR}/include           # udm_version.h
    ${TP_INC}
    ${CGNS_INC}
    ${ZOLTAN_INC}
)


link_directories(
      ${TP_LIB}
      ${CGNS_LIB}
      ${ZOLTAN_LIB}
)


set(udm_files
    config/UdmConfigBase.cpp
    config/UdmDfiConfig.cpp
    config/UdmDfiValue.cpp
    config/UdmDomainConfig.cpp
    config/UdmFileInfoConfig.cpp
    config/UdmFilePathConfig.cpp
    config/UdmFlowSolutionListConfig.cpp
    config/UdmMpiConfig.cpp
    config/UdmProcessConfig.cpp
    config/UdmSettingsConfig.cpp
    config/UdmSolutionFieldConfig.cpp
    config/UdmTimeSliceConfig.cpp
    config/UdmUnitListConfig.cpp
    model/UdmBar.cpp
    model/UdmCell.cpp
    model/UdmComponent.cpp
    model/UdmCoordsValue.cpp
    model/UdmElements.cpp
    model/UdmEntity.cpp
    model/UdmEntityVoxels.cpp
    model/UdmFlowSolutions.cpp
    model/UdmGeneral.cpp
    model/UdmGlobalRankid.cpp
    model/UdmGridCoordinates.cpp
    model/UdmInfo.cpp
    model/UdmIterativeData.cpp
    model/UdmModel.cpp
    model/UdmNode.cpp
    model/UdmRankConnectivity.cpp
    model/UdmSections.cpp
    model/UdmShell.cpp
    model/UdmSolid.cpp
    model/UdmSolutionData.cpp
    model/UdmUserDefinedDatas.cpp
    model/UdmZone.cpp
    partition/UdmLoadBalance.cpp
    utils/UdmScannerCells.cpp
    utils/UdmSearchTable.cpp
    utils/UdmSerialization.cpp
    utils/UdmStopWatch.cpp
    UdmBase.cpp
    UdmErrorHandler.cpp
    udmlib_cc.cpp
)


if(NOT with_MPI)
  set(udm_target UDM)
  set(udm_libs "-lTP -lzoltan -lcgns")
else()
  set(udm_target UDMmpi)
  set(udm_libs "-lTPmpi -lzoltan -lcgns")
endif()


add_library(${udm_target} STATIC ${udm_files})
target_link_libraries(${udm_target} ${udm_libs})
install(TARGETS ${udm_target} DESTINATION lib)


install(FILES
        ${PROJECT_SOURCE_DIR}/include/udm_define.h
        ${PROJECT_SOURCE_DIR}/include/udm_errorno.h
        ${PROJECT_SOURCE_DIR}/include/udm_memutils.h
        ${PROJECT_SOURCE_DIR}/include/udm_mpiutils.h
        ${PROJECT_SOURCE_DIR}/include/udm_pathutils.h
        ${PROJECT_SOURCE_DIR}/include/udm_strutils.h
        ${PROJECT_SOURCE_DIR}/include/UdmBase.h
        ${PROJECT_SOURCE_DIR}/include/UdmErrorHandler.h
        ${PROJECT_SOURCE_DIR}/include/udmlib.h
        ${PROJECT_SOURCE_DIR}/include/config/UdmFlowSolutionListConfig.h
        ${PROJECT_SOURCE_DIR}/include/config/UdmSettingsConfig.h
        ${PROJECT_SOURCE_DIR}/include/config/UdmDfiKeywords.h
        ${PROJECT_SOURCE_DIR}/include/config/UdmMpiConfig.h
        ${PROJECT_SOURCE_DIR}/include/config/UdmDfiConfig.h
        ${PROJECT_SOURCE_DIR}/include/config/UdmUnitListConfig.h
        ${PROJECT_SOURCE_DIR}/include/config/UdmDomainConfig.h
        ${PROJECT_SOURCE_DIR}/include/config/UdmDfiValue.h
        ${PROJECT_SOURCE_DIR}/include/config/UdmFilePathConfig.h
        ${PROJECT_SOURCE_DIR}/include/config/UdmFileInfoConfig.h
        ${PROJECT_SOURCE_DIR}/include/config/UdmSolutionFieldConfig.h
        ${PROJECT_SOURCE_DIR}/include/config/UdmTimeSliceConfig.h
        ${PROJECT_SOURCE_DIR}/include/config/UdmProcessConfig.h
        ${PROJECT_SOURCE_DIR}/include/config/UdmConfigBase.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmFlowSolutions.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmShell.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmUserDefinedDatas.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmEntity.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmRankConnectivity.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmCell.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmNode.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmSections.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmGridCoordinates.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmInfo.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmIterativeData.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmElements.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmSolutionData.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmGlobalRankid.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmGeneral.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmEntityVoxels.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmSolid.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmBar.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmZone.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmCoordsValue.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmModel.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmICellComponent.h
        ${PROJECT_SOURCE_DIR}/include/model/UdmComponent.h
        ${PROJECT_SOURCE_DIR}/include/partition/UdmLoadBalance.h
        ${PROJECT_SOURCE_DIR}/include/utils/UdmStopWatch.h
        ${PROJECT_SOURCE_DIR}/include/utils/UdmScannerCells.h
        ${PROJECT_SOURCE_DIR}/include/utils/UdmSerialization.h
        ${PROJECT_SOURCE_DIR}/include/utils/UdmSearchTable.h
        ${PROJECT_BINARY_DIR}/include/udm_version.h
        DESTINATION include
)
