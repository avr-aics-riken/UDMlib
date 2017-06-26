


# Checks for libraries.
ORG_CFLAGS="$CFLAGS"
ORG_CXXFLAGS="$CXXFLAGS"
ORG_LDFLAGS="$LDFLAGS"
ORG_LIBS="$LIBS"


###################################
# Check Zoltan library
#

        ZOLTAN_LIBS="-lzoltan"


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

UDM_LIBS="$UDM_LIBS"" -lm"

CFLAGS="$ORG_CFLAGS"
CXXFLAGS="$ORG_CXXFLAGS"
LDFLAGS="$ORG_LDFLAGS"
LIBS="$ORG_LIBS"





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
