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

# - Try to find UDMlib
# Once done, this will define
#
#  UDM_FOUND - system has UDMlib
#  UDM_INCLUDE_DIRS - UDMlib include directories
#  UDM_LIBRARIES - link these to use UDMlib

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(UDM_PKGCONF UDM)

if(CMAKE_PREFIX_PATH)
  set(UDM_CANDIDATE_PATH ${CMAKE_PREFIX_PATH})
  file(GLOB tmp "${CMAKE_PREFIX_PATH}/[Jj][Hh][Pp][Cc][Nn][Dd][Ff]*/")
  list(APPEND UDM_CANDIDATE_PATH ${tmp})
endif()

# Include dir
find_path(UDM_INCLUDE_DIR
  NAMES udm_define.h
  PATHS ${UDM_ROOT} ${UDM_PKGCONF_INCLUDE_DIRS} ${UDM_CANDIDATE_PATH}
  PATH_SUFFIXES include
)

# Finally the library itself
find_library(UDM_LIBRARY
  NAMES UDM
  PATHS ${UDM_ROOT} ${UDM_PKGCONF_LIBRARY_DIRS} ${UDM_CANDIDATE_PATH}
  PATH_SUFFIXES lib
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(UDM_PROCESS_INCLUDES UDM_INCLUDE_DIR)
set(UDM_PROCESS_LIBS UDM_LIBRARY)
libfind_process(UDM)
