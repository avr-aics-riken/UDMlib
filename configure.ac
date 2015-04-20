#
# UDMlib - Unstructured Data Management Library
# Copyright (C) 2012-2015 Institute of Industrial Science, The University of Tokyo.
# All rights reserved.
#
#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.


AC_PREREQ(2.57)
AC_INIT([UDMlib], [1.0.7],[keno@{iis.u-tokyo.ac, riken}.jp],[UDMlib])

AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR([src/UdmBase.cpp])
AC_CONFIG_HEADERS([config.h])

#
# revision No.
#
UDM_REVISION="20150420_1200"
AC_SUBST(UDM_REVISION)


###############################################
# setting default prefix
#
AC_PREFIX_DEFAULT([usr/local/udmlib])
UDMLIB_INSTALL_DIR=${INSTALLDIR:-'/usr/local/udmlib'}
if test x"$prefix" = x"NONE" ; then
  prefix=$UDMLIB_INSTALL_DIR
else
  UDMLIB_INSTALL_DIR="$prefix"
fi
AC_PREFIX_DEFAULT(prefix)

#
# compiler
#
AC_ARG_WITH(comp, [AS_HELP_STRING([--with-comp=(INTEL|FJ|GNU)],[Specify Compiler type])], , with_comp=none)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_FC
AC_PROG_RANLIB

# Checks for libraries.
ORG_CFLAGS="$CFLAGS"
ORG_CXXFLAGS="$CXXFLAGS"
ORG_LDFLAGS="$LDFLAGS"
ORG_LIBS="$LIBS"

#
# MPI setting. wrappers like mpicxx, mpicc, mpiCC, mpic++, mpiFCCpx are taking
# into consideration, if not add in case.
#

udm_enable_mpi="none"

UDM_CXX="$CXX"
AC_SUBST(UDM_CXX)

case "$UDM_CXX" in
  *mpicxx)   udm_enable_mpi="wrappermpi" ;;
  *mpicc)    udm_enable_mpi="wrappermpi" ;;
  *mpiCC)    udm_enable_mpi="wrappermpi" ;;
  *mpic++)   udm_enable_mpi="wrappermpi" ;;
  *mpiFCCpx) udm_enable_mpi="wrappermpi" ;;
  *mpiicc)   udm_enable_mpi="wrappermpi" ;;
  *mpiicpc)  udm_enable_mpi="wrappermpi" ;;
  CC)       udm_enable_mpi="wrappermpi" ;;
esac

# mpiFCCpx ; Fujitsu  FX
# CC       ; NERSC Hopper

###############################################
# configure MPI
AC_SUBST(MPICH_DIR)
AC_SUBST(MPICH_FLAGS)
AC_SUBST(MPICH_LDFLAGS)
AC_SUBST(MPICH_LIBS)
AC_SUBST(MPICH_FCLIBS)


#
# Without MPI Library
#
AC_ARG_ENABLE([mpi],
           [AS_HELP_STRING([--disable-mpi],
                           [without MPI library and Zoltan library])],
           [enable_mpi="without_mpi"])

if test x"$enable_mpi" = x"without_mpi" ; then
    udm_enable_mpi="without_mpi"
fi

#
# if wrapper compiler is specified, skip this part
#

if test x"$udm_enable_mpi" = x"none" ; then

###############################################
# Setting MPICH environment
#
    AC_ARG_WITH(mpich,
        AS_HELP_STRING(
            [--with-mpich=DIR],
            [Specify MPICH install directory.]
        ), ,
        with_mpich=none)
    if test x"$with_mpich" != x"none" ; then
      MPICH_DIR=$with_mpich;
      MPICH_FLAGS="-D_HAS_MPICH_ -I$MPICH_DIR/include"
      MPICH_LDFLAGS=-L$MPICH_DIR/lib
      udm_enable_mpi="mpich"
    fi

###############################################
# Setting OpenMPI environment
#
    AC_ARG_WITH(ompi,
        AS_HELP_STRING(
            [--with-ompi=DIR],
            [Specify OpenMPI install directory.]
        ), ,
        with_ompi=none)
    if test x"$with_ompi" != x"none" ; then
      MPICH_DIR=$with_ompi;
      MPICH_FLAGS="-D_HAS_OMPI_ -I$MPICH_DIR/include"
      MPICH_LDFLAGS=-L$MPICH_DIR/lib
      udm_enable_mpi="ompi"
      MPICH_LIBS="-lmpi -lmpi_cxx"
    fi
    if test -z $MPICH_DIR ; then
      AC_MSG_ERROR([MPICH library is required in configure options (--with-mpich or --with-ompi)!])
    fi

    # check mpich library
    CFLAGS="$CFLAGS $MPICH_FLAGS"
    LDFLAGS="$LDFLAGS $MPICH_LDFLAGS"
    AC_CHECK_LIB([mpi], [MPI_Init], [],
                   [AC_MSG_ERROR([MPICH library is required, but not found!])])

elif test x"$udm_enable_mpi" = x"without_mpi" ; then
    MPICH_FLAGS="-DWITHOUT_MPI"
fi

#
# MPI Library for fortran
#

if test x"$with_comp" = x"FJ" ; then
    MPICH_LIBS=""
    case "$host" in
    sparc*)
        MPICH_FCLIBS="$MPICH_LIBS"" -lmpi_f77 -lmpi_f90"
    ;;
    esac
elif test x"$udm_enable_mpi" = x"mpich" ; then
    MPICH_LIBS="-lmpi -lmpicxx -lpthread"
    MPICH_FCLIBS="$MPICH_LIBS"" -lfmpich"
elif test x"$udm_enable_mpi" = x"ompi" ; then
    MPICH_LIBS="-lmpi -lmpi_cxx"
    MPICH_FCLIBS="$MPICH_LIBS"" -lmpi_f77 -lmpi_f90"
fi

# warning. if a wrapper compiler is used, "udm_enable_mpi" is empty, but parallel.
#
if test x"$udm_enable_mpi" = x"none" ; then
  echo "  Warning: Neither MPICH nor OpenMPI library is not specified."
  echo "           If compilation error occurs about MPI, specify --with-mpich or --with-ompi option."
fi

###################################
# Check CGNS library
#
AC_SUBST(CGNS_DIR)
AC_SUBST(CGNS_FLAGS)
AC_SUBST(CGNS_LDFLAGS)
AC_SUBST(CGNS_LIBS)
AC_ARG_WITH(cgns,
    AS_HELP_STRING(
        [--with-cgns=DIR],
        [Specify CGNS install directory.]
    ), ,
    AC_MSG_ERROR([CGNS library is required in configure option (--with-cgns)!])
    )
if test -e "$with_cgns" ; then
    CGNS_DIR=$with_cgns;
    CGNS_FLAGS=" -I$CGNS_DIR/include"
    CGNS_LDFLAGS=-L$CGNS_DIR/lib
    CGNS_LIBS="-lcgns"

else
    AC_MSG_ERROR([not found CGNS DIR : $with_cgns!])
fi
# check CGNS libray
CFLAGS="$CFLAGS $CGNS_FLAGS"
LDFLAGS="$LDFLAGS $CGNS_LDFLAGS"
# AC_CHECK_LIB([cgns], [cg_open], [],
#               [AC_MSG_ERROR([CGNS library is required, but not found!])])

###################################
# Check TextParser library
#
AC_SUBST(TP_DIR)
AC_SUBST(TP_FLAGS)
AC_SUBST(TP_LDFLAGS)
AC_SUBST(TP_LIBS)
AC_ARG_WITH(tp,
    AS_HELP_STRING(
        [--with-tp=DIR],
        [Specify TextParser install directory.]
    ), ,
    AC_MSG_ERROR([TextParser library is required in configure option (--with-tp)!])
    )
if test -e "$with_tp" ; then
    if test -f "$with_tp" ; then
        TP_CONFIG="$with_tp"
    else
        TP_CONFIG="$with_tp"/bin/tp-config
    fi
    if test -e "$TP_CONFIG" ; then
        TP_FLAGS="`$TP_CONFIG --cflags`"
        TP_LDFLAGS="`$TP_CONFIG --libs`"
    else
        TP_DIR=$with_tp;
        TP_FLAGS=" -I$TP_DIR/include"
        TP_LDFLAGS="-L$TP_DIR/lib -lTP"
    fi
else
    AC_MSG_ERROR([not found TextParser DIR : $with_tp!])
fi

# check TextParser libray
CFLAGS="$CFLAGS $TP_FLAGS"
LDFLAGS="$LDFLAGS $TP_LDFLAGS"
# AC_CHECK_LIB([TP], [read], [],
#               [AC_MSG_ERROR([TextParser library is required, but not found!])])

###################################
# Check Zoltan library
#
AC_SUBST(ZOLTAN_DIR)
AC_SUBST(ZOLTAN_FLAGS)
AC_SUBST(ZOLTAN_LDFLAGS)
AC_SUBST(ZOLTAN_LIBS)

if test x"$udm_enable_mpi" != x"without_mpi" ; then
    AC_ARG_WITH(zoltan,
        AS_HELP_STRING(
            [--with-zoltan=DIR],
            [Specify Zoltan install directory.]
        ), ,
        AC_MSG_ERROR([Zoltan library is required in configure option (--with-zoltan)!])
        )
    if test -e "$with_zoltan" ; then
        ZOLTAN_DIR=$with_zoltan;
        ZOLTAN_FLAGS=" -I$ZOLTAN_DIR/include"
        ZOLTAN_LDFLAGS=-L$ZOLTAN_DIR/lib
        ZOLTAN_LIBS="-lzoltan"
    else
        AC_MSG_ERROR([not found Zoltan DIR : $with_zoltan!])
    fi

    # check Zoltan libray
    CFLAGS="$CFLAGS $ZOLTAN_FLAGS"
    LDFLAGS="$LDFLAGS $ZOLTAN_LDFLAGS"
    #AC_CHECK_LIB([zoltan], [Zoltan_Create], [],
    #               [AC_MSG_ERROR([Zoltan library is required, but not found!])])
fi

#
# f90 example
#
# AC_ARG_WITH(f90example,
#              [AS_HELP_STRING(
#                    [--with-f90example=(yes|no)],
#                    [make f90 example [yes]])],
#              , with_f90example=no)

#
# f90 compiler check
#
#if test x"_$FC" = x"_"; then
#  if test x"_$F90" = x"_"; then
#    with_f90example="no"
#  else
#    FC=$F90
#  fi
#fi
#if test x"_$FCFLAGS" = x"_"; then
#  if test x"_$F90FLAGS" != x"_"; then
#    FCFLAGS=$F90FLAGS
#  fi
#fi

AC_SUBST(UDM_FLAGS)
AC_SUBST(UDM_LDFLAGS)
AC_SUBST(UDM_LIBS)
AC_SUBST(UDM_SYMBOLS)
UDM_FLAGS='-I$(top_builddir)/include'
UDM_LDFLAGS='-L$(top_builddir)/lib'
UDM_LIBS='-ludm'

UDM_SYMBOLS=''
#
# UdmSize_t Datatype
#
AC_ARG_ENABLE([size8],
           [AS_HELP_STRING([--enable-size8],
                           [Specify as 64bit of UdmSize_t])],
           [enable_size8="enable_size8"])
if test x"$enable_size8" = x"enable_size8" ; then
    UDM_SYMBOLS="$UDM_SYMBOLS -DUDM_SIZE64"
fi

#
# real8 in cc interface Datatype
#
AC_ARG_ENABLE([real8],
           [AS_HELP_STRING([--enable-real8],
                           [Specify as double of real])],
           [enable_real8="enable_real8"])
if test x"$enable_real8" = x"enable_real8" ; then
    UDM_SYMBOLS="$UDM_SYMBOLS -DUDM_REAL8"
fi

#
# int8 in cc interface Datatype
#
AC_ARG_ENABLE([int8],
           [AS_HELP_STRING([--enable-int8],
                           [Specify as long long of integer])],
           [enable_int8="enable_int8"])
if test x"$enable_int8" = x"enable_int8" ; then
    UDM_SYMBOLS="$UDM_SYMBOLS -DUDM_INT8"
fi
UDM_FLAGS="$UDM_FLAGS $UDM_SYMBOLS"

UDM_FC="$FC"
AC_SUBST(UDM_FC)

UDM_FC_LD="$FC"
AC_SUBST(UDM_FC_LD)

if test x"$with_comp" = x"FJ"; then
  UDM_FC_LD="$CXX"" --linkfortran"
elif test x"$with_comp" = x"INTEL" ; then
  UDM_LIBS="$UDM_LIBS"" -lstdc++"
elif test x"$with_comp" = x"GNU" ; then
  UDM_LIBS="$UDM_LIBS"" -lstdc++"
elif test x"$with_comp" = x"none" ; then
  UDM_LIBS="$UDM_LIBS"" -lstdc++"
fi
UDM_LIBS="$UDM_LIBS"" -lm"

CFLAGS="$ORG_CFLAGS"
CXXFLAGS="$ORG_CXXFLAGS"
LDFLAGS="$ORG_LDFLAGS"
LIBS="$ORG_LIBS"

#
# udm-config special flags
#
UDM_CFG_CFLAGS="-I$UDMLIB_INSTALL_DIR/include"" $UDM_SYMBOLS $MPICH_FLAGS $CGNS_FLAGS $TP_FLAGS $ZOLTAN_FLAGS"
AC_SUBST(UDM_CFG_CFLAGS)

UDM_CFG_LDFLAGS="-L$UDMLIB_INSTALL_DIR/lib"" $ZOLTAN_LDFLAGS $CGNS_LDFLAGS $MPICH_LDFLAGS"
AC_SUBST(UDM_CFG_LDFLAGS)

UDM_CFG_LIBS="$UDM_LIBS $ZOLTAN_LIBS $CGNS_LIBS $TP_LDFLAGS"
AC_SUBST(UDM_CFG_LIBS)

UDM_CFG_CXX="$CXX"
AC_SUBST(UDM_CFG_CXX)

UDM_CFG_FC="$FC"
AC_SUBST(UDM_CFG_FC)

UDM_CFG_FC_LD="$FC"
AC_SUBST(UDM_CFG_FC_LD)

################################
# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h \
                  float.h \
                  limits.h \
                  malloc.h \
                  stdlib.h \
                  string.h \
                  strings.h \
                  sys/param.h \
                  sys/socket.h \
                  sys/time.h \
                  unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_TYPE_SIGNAL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([atexit floor getcwd gethostname gettimeofday memmove memset mkdir realpath strcasecmp strchr strncasecmp strstr])

#
# MAKE_SUB_DIRS
#
AC_SUBST(MAKE_SUB_DIRS)
MAKE_SUB_DIRS="src \
               doc \
               tools/udm-frm/src"
if test x"$udm_enable_mpi" != x"without_mpi" ; then
  MAKE_SUB_DIRS="$MAKE_SUB_DIRS"" \
                examples/cxx/partition \
                examples/cxx/cgns \
                examples/cxx/timeslice \
                examples/cxx/dfi  \
                examples/cc/partition \
                examples/cc/cgns \
                examples/cc/timeslice \
                examples/cc/dfi"
fi
if test x"$with_f90example" = x"yes" ; then
  MAKE_SUB_DIRS="$MAKE_SUB_DIRS"" examples/f90_code"
fi

AC_CONFIG_FILES([Makefile \
                src/Makefile \
                examples/cxx/partition/Makefile \
                examples/cxx/cgns/Makefile \
                examples/cxx/timeslice/Makefile \
                examples/cxx/dfi/Makefile \
                examples/cc/partition/Makefile \
                examples/cc/cgns/Makefile \
                examples/cc/timeslice/Makefile \
                examples/cc/dfi/Makefile \
                tools/udm-frm/src/Makefile \
                udm-config \
                include/udm_version.h \
                doc/Makefile])

AC_OUTPUT

chmod +x ./udm-config
