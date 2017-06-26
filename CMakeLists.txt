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
#
#-*- CMAKE -*-#
# Process this file with cmake to produce a configure script.

# Following parameters must be given at cmake command line

# Build options, see Readme.md for details.
#
# -D INSTALL_DIR=install_directory
#
# -D with_MPI={yes|no}
#
# -D with_TP=installed_directory
#
# -D with_CGNS=installed_directory
#
# -D with_HDF5=installed_directory
#
# -D with_ZOLTAN=installed_directory
#
# -D with_util={yes|no}
#
# -D with_example={no|yes}
#
# -D real_type={float|double}
#

cmake_minimum_required(VERSION 2.6)


#######
# Build options
#######

option (with_util "Enable utility" "ON")
option (with_MPI "Enable MPI" "ON")
option (with_example "Compiling examples" "OFF")
option (real_type "Type of floating point" "OFF")


#######
# Project setting
#######

project(UDMlib)

if(INSTALL_DIR STREQUAL "")
  set(INSTALL_DIR "/usr/local/UDMlib")
endif()

# project()でCMAKE_INSTALL_PREFIX=/usr/localとなるので、再定義
set (CMAKE_INSTALL_PREFIX "${INSTALL_DIR}")

#set(CMAKE_BUILD_TYPE Release)



#######
set(PROJECT_VERSION "1.2.0")
set(LIB_REVISION "20170622_1200")

message(" ")
message("PROJECT : UDMlib " ${PROJECT_VERSION} " : " ${LIB_REVISION})
message(" ")

#######



#######
# Cmake module
#######

set(CMAKE_MODULE_PATH  ${PROJECT_SOURCE_DIR}/cmake)
include(${PROJECT_SOURCE_DIR}/cmake/CompileOptionSelector.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/udm_package.cmake)



#######
# Options
#######


AddOptimizeOption()

#checkOpenMP()

# Real type
precision()

# Serial
if(NOT with_MPI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWITHOUT_MPI")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWITHOUT_MPI")
endif()


#######
# Display options
#######

message(" ")
message( STATUS "Destination PATH  : "      ${CMAKE_INSTALL_PREFIX})
message( STATUS "CMAKE_C_COMPILER  : "      ${CMAKE_C_COMPILER})
message( STATUS "CMAKE_C_FLAGS     : "      ${CMAKE_C_FLAGS})
message( STATUS "CMAKE_CXX_COMPILER: "      ${CMAKE_CXX_COMPILER})
message( STATUS "CMAKE_CXX_FLAGS   : "      ${CMAKE_CXX_FLAGS})

message(" ")
message( STATUS "Type of floating point : "  ${real_type})
message( STATUS "Utilities              : "  ${with_util})
message( STATUS "MPI support            : "  ${with_MPI})
message( STATUS "Example                : "  ${with_example})
message(" ")

if(USE_F_TCS STREQUAL "YES")
else()
  if(with_MPI)
    include(FindMPI)

    if(MPI_CXX_FOUND)
      message(" ")
      message("MPI-CXX-CMP  : ${MPI_CXX_COMPILER}")
      message("MPI-CXX-FLAGS: ${MPI_CXX_COMPILE_FLAGS}")
      message("MPI-CXX-INC  : ${MPI_CXX_INCLUDE_PATH}")
      message("MPI-LNK-FLAGS: ${MPI_CXX_LINK_FLAGS}")
      message("MPI-LIBS     : ${MPI_CXX_LIBRARIES}")
      message(" ")
    endif(MPI_CXX_FOUND)
  endif()
endif()


#######
# include PATH
#######

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
)


#######
# Find Libraries to depend on
#######

# TextParser
IF(with_TP)
  SET(TP_DIR "${with_TP}")
  SET(TP_INC "${TP_DIR}/include")
  SET(TP_LIB "${TP_DIR}/lib")
ELSE()
  MESSAGE("Error: can not find TextParser.")
ENDIF()


# CGNS
IF(with_CGNS)
  SET(CGNS_DIR "${with_CGNS}")
  SET(CGNS_INC "${CGNS_DIR}/include")
  SET(CGNS_LIB "${CGNS_DIR}/lib")
ELSE()
  MESSAGE("Error: can not find CGNS library.")
ENDIF()


# HDF5
IF(with_HDF5)
  SET(HDF5_DIR "${with_HDF5}")
  SET(HDF5_INC "${HDF5_DIR}/include")
  SET(HDF5_LIB "${HDF5_DIR}/lib")
ELSE()
  MESSAGE("Error: can not find HDF5 library.")
ENDIF()


# ZOLTAN
IF(with_ZOLTAN)
  SET(ZOLTAN_DIR "${with_ZOLTAN}")
  SET(ZOLTAN_INC "${ZOLTAN_DIR}/include")
  SET(ZOLTAN_LIB "${ZOLTAN_DIR}/lib")
ELSE()
  MESSAGE("Error: can not find ZOLTAN.")
ENDIF()


add_definitions(-DHAVE_CONFIG_H)

message(" ")
message( STATUS "TextParser support     : "  ${with_TP})
message( STATUS "CGNS support           : "  ${with_CGNS})
message( STATUS "HDF5 support           : "  ${with_HDF5})
message( STATUS "ZOLTAN support         : "  ${with_ZOLTAN})
message(" ")



#######
# tools
#######

if(with_util)
  if(UDM_CROSS_OPTION)
    message(" ")
    message("frm        : not build because cross compilation is assumed")
  else()
    message("frm        : build")
  endif()
endif()



#######
# Example
#######

if (with_example)
  enable_testing()
  message("Code test will be performed")
else()
  message("Code test is skipped")
endif()



#######
# Check header files
#######

include(CheckIncludeFiles)

CHECK_INCLUDE_FILES(inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILES(memory.h HAVE_MEMORY_H)
CHECK_INCLUDE_FILES(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILES(stdlib.h HAVE_STDLIB_H)
CHECK_INCLUDE_FILES(strings.h HAVE_STRINGS_H)
CHECK_INCLUDE_FILES(string.h HAVE_STRING_H)
CHECK_INCLUDE_FILES(sys/stat.h HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILES(sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)



#######
# Sub Directories
#######

add_subdirectory(src)

if(with_util)
  add_subdirectory(tools/udm-frm)
endif()

add_subdirectory(doc)

if(with_example)
  if (NOT with_MPI)
    message("Examples require MPI")
  else()
    add_subdirectory(examples/cc)
    add_subdirectory(examples/cxx)
  endif()
endif()


#######
# configure files
#######

configure_file( config.h.in.cmake config.h )
configure_file( include/udm_version.h.in ${PROJECT_BINARY_DIR}/include/udm_version.h @ONLY)


#######
# Install files
#######

install(FILES  ChangeLog.md
               Readme.md
               License.txt
               ${PROJECT_BINARY_DIR}/CMakeCache.txt
        DESTINATION share)


#######
# Installer
#######

udm_package()
