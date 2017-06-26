# UDMlib - Unstructured Data Management library

# Copyright (c) 2012-2015 Institute of Industrial Science, The University of Tokyo. All rights reserved.
# Copyright (c) 2015-2017 Advanced Institute for Computational Science, RIKEN. All rights reserved.
# Copyright (c) 2017 Research Institute for Information Technology (RIIT), Kyushu University. All rights reserved.


## OUTLINE

UDMlib is a file management library for unstructured grid data, and provides the following functions:

- Management of distributed files
- Restart from previous calculated data for the same number of processes (standard)
- Restart from previous calculated data for the different number of processes
- Staging helper


## SOFTWARE REQUIREMENT
- Cmake
- MPI library
- CGNS http://cgns.sourceforge.net/
- HDF5
- TextParser
- Zoltan http://www.cs.sandia.gov/Zoltan/Zoltan.html
   - The same MPI library must be employed to build for both Zoltan and UDMlib.


## INGREDIENTS
~~~
ChangeLog.md      History of development
CMakeLists.txt.   Makefile of Cmake
License.txt       License to apply
Readme.md         This document, including the description of build
cmake/            Modules of cmake
doc/              Document
example/          Example source codes
include/          Header files
src/              Source codes
tools/udm-frm     File Rank Mapper
~~~


## HOW TO BUILD

### Build

~~~
$ export UDM_HOME=/hogehoge
$ mkdir build
$ cd build
$ cmake [options] ..
$ make
$ sudo make install
~~~


### Options

`-D INSTALL_DIR=` *Install_directory*

>  Specify the directory where the UDMlib library will be installed. The main library will be installed at `install_directory/lib` and the header files will be placed at `install_directory/include`. The default installation directory is `/usr/local/UDMlib`.

`-D with_util=` {yes | no}

>  Specify whether or not to build the utility tools. The default option is yes.
>
>  In the case of cross-compilation, the frm tool (for supporting the staging process) will not be built. On the other hand, in the case of native compile environment, when setting `-D with_util=yes` then all the utility tools will be built along with the main UDMlib library.
>
>  In addition, if `with_MPI=no` is set, then fconv and upacs2dfi tools will not be built.
>  

`-D with_MPI=` {yes | no}

>  If you intend to use an MPI library, then specify `with_MPI=yes`. The default option is yes.

`-D with_TP =` *TextParser_directory*

> Specify the directory path where the TextParser library is installed.

`-D with_CGNS=` *CGNS_directory*

> Specify the directory path where the CGNS library is installed.

`-D with_HDF5=` *HDF5_directory*

> Specify the directory path where the HDF5 library is installed.

`-D with_SZIP_LIB=` *SZIP_directory*

> Specify the directory path where the SZIP library is installed (Szip compression enabled HDF5).

`-D with_Z_LIB=` *ZLIB_directory*

> Specify the directory path where the ZLIB library is installed (Zlib compression enabled HDF5).

`-D with_ZOLTAN=` *Zoltan_directory*

> Specify the directory path where the Zoltan library is installed. The Zoltan library must be the compiled with the same compiler set to build the UDMlib library.

`-D with_example=` {no | yes}

>  This option will enable the compiling of testing codes. The default is no.


The default compiler options are described in `cmake/CompilerOptionSelector.cmake` file. See BUILD OPTION section in CMakeLists.txt in detail.


## Configure Examples

`$ export UDM_HOME=hogehoge`

In the following configuration examples, it is assumed that the TextParser, CGNS, and Zoltan libraries are installed under the UDM_HOME directory. If not, please adjust the directory path to reflect your environment.


### INTEL/GNU compiler

~~~
$ cmake -DINSTALL_DIR=${UDM_HOME}/UDMlib \
        -Dwith_MPI=yes \
        -Dreal_type=float \
        -Dwith_util=no \
        -Dwith_example=no \
        -Dwith_TP=${UDM_HOME}/TextParser \
        -Dwith_ZOLTAN=${UDM_HOME}/Zoltan \
        -Dwith_CGNS=${UDM_HOME}/CGNS \
        -Dwith_HDF5=${UDM_HOME}/HDF5 ..
~~~


### FUJITSU compiler / FX10, FX100, K on login nodes (Cross compilation)

~~~
$ cmake -DINSTALL_DIR=${UDM_HOME}/UDMlib \
            -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain_fx10.cmake \
            -Dwith_MPI=yes \
	    -Dreal_type=float \
            -Dwith_util=no \
            -Dwith_example=no \
            -Dwith_TP=${UDM_HOME}/TextParser \
            -Dwith_ZOLTAN=${UDM_HOME}/Zoltan \
            -Dwith_CGNS=${UDM_HOME}/CGNS \
            -Dwith_HDF5=${UDM_HOME}/HDF5 ..

$ cmake -DINSTALL_DIR=${UDM_HOME}/UDMlib \
            -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain_fx100.cmake \
            -Dwith_MPI=yes \
	    -Dreal_type=float \
            -Dwith_example=no \
            -Dwith_util=no \
            -Dwith_TP=${UDM_HOME}/TextParser \
            -Dwith_ZOLTAN=${UDM_HOME}/Zoltan \
            -Dwith_CGNS=${UDM_HOME}/CGNS \
            -Dwith_HDF5=${UDM_HOME}/HDF5 ..

$ export cmake-bin=/opt/local/bin/cmake 
$ cmake-bin -DINSTALL_DIR=${UDM_HOME}/UDMlib \
           -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain_K.cmake \
           -Dreal_type=float \
           -Dwith_MPI=yes \
           -Dwith_example=no \
           -Dwith_util=no \
           -Dwith_TP=${UDM_HOME}/TextParser \
           -Dwith_ZOLTAN=${UDM_HOME}/Zoltan \
           -Dwith_CGNS=${UDM_HOME}/CGNS \
           -Dwith_HDF5=/opt/aics/netcdf/k \
           -Dwith_Z_LIB=/usr/lib64 \
           -Dwith_SZIP_LIB=/opt/aics/netcdf/k/lib \
           ../
~~~


##### Note
- On Fujitsu machines(fx10, K, fx100), confirm the appropriate directrory paths for the selected compiler environment.
- Before the building process, it is advised to execute the following command: `$ make distclean`



### Compilation of staging tool (frm)

If you desire to install the staging tool onto the front-end login node via cross-compiling environment, this tool must be compiled via native GNU compiler provided at the login node. If the front-end login node does not have any MPI library, then set `-Dwith_MPI=no`.



## Note for building required libraries


### Zlib

~~~
$ tar xvfz zlib-x.x.x.tar.gz
$ cd zlib-x.x.x
$ export CC=icc && ./configure --prefix=${UDM_HOME}/Zlib
$ make && make install
~~~

* In the case of Futjitsu compiler, set CC=fccpx
* On the K computer, this library is available at "/usr/lib64"


### SZIP

~~~
$ tar xvfz szip-x.x.x.tar.gz
$ cd szip-x.x.x
$ export CC=icc CFLAGS=-O3
$ export CXX=icpc CXXFLAGS=-O3
$ ./configure --prefix=${UDM_HOME}/SZIP
$ make && make install
~~~

* In the case of Fujitsu compiler, set CC=fccpx, CXX=FCCpx, and add --host=x86_64-unknown-linux-gnu
* On the K computer, this library is available at "/opt/aics/netcdf".


### HDF5

~~~
$ tar xvfz hdf5-x.x.x.tar.gz
$ cd hdf5-x.x.x
$ export CC=icc CFLAGS=-O3
$ export CXX=icpc CXXFLAGS=-O3
$ export FC=ifort FCFLAGS=-O3
$ ./configure --prefix=${UDM_HOME}/HDF5 --enable-64bit \
              --enable-fortran \
              --with-zlib=${UDM_HOME}/Zlib/include,${UDM_HOME}/Zlib/lib \
              --enable-cxx \
              --with-szip=${UDM_HOME}/SZIP
$ make && make install
~~~

* In the case of Fujitsu compiler, set CC=fccpx, CXX=FCCpx, FC=frtpx, and add --host=x86_64-unknown-linux-gnu
* On the K computer, this library is available at "/opt/aics/netcdf".


### CGNS

-  Before building the CGNS library, you might be needed to move the header file zlib.h into the libz.a directory (lib/ directory).

~~~
For INTEL/GNU compiler

$ tar xzvf cgnslib_x.x.x.tar.gz
$ cd cgnslib_x.x.x/src
$ export CC=mpicc CFLAGS=-O3
$ export FC=mpif90 FCFLAGS=-O3
$ export HDF5_LIBRARY_DIR=${UDM_HOME}/ompi-intel/HDF5/lib
$ export HDF5_INCLUDE_DIR=${UDM_HOME}/ompi-intel/HDF5/include
$ ./configure --prefix=$1 \
              --enable-64bit \
              --with-fortran \
              --with-hdf5=${UDM_HOME}/HDF5 \
              --with-szip=${UDM_HOME}/SZIP/lib/libsz.a \
              --with-zlib=${UDM_HOME}/Zlib/lib/libz.a
$ make && make install
~~~

~~~
Configure option for the K computer

/opt/local/bin/cmake \
             -DCMAKE_C_COMPILER=mpifccpx \
             -DCMAKE_C_FLAGS="-Xg -Kfast -KPIC" \
             -DCMAKE_INSTALL_PREFIX=${UDM_HOME}/CGNS \
             -DCGNS_ENABLE_64BIT=On \
             -DCGNS_BUILD_SHARED=Off \
             -DCGNS_ENABLE_HDF5=On \
             -DHDF5_LIBRARY=/opt/aics/netcdf/k/lib/libhdf5.a \
             -DHDF5_INCLUDE_PATH=/opt/aics/netcdf/k/include \
             -DHDF5_NEED_SZIP=On \
             -DHDF5_NEED_ZLIB=On \
             -DSZIP_LIBRARY=/opt/aics/netcdf/k/lib/libsz.a \
             -DZLIB_LIBRARY=/usr/lib64/libz.so \
             ../
~~~


### Zoltan

~~~
$ tar xvfz zoltan_distrib_v3.81.tar.gz
$ cd Zoltan_v3.81
$ mkdir BUILD_DIR
$ cd BUILD_DIR
$ export CC=mpicc CFLAGS=-O3 FC=mpif90 FCFLAGS=-O3
$ ../configure --prefix=${UDM_HOME}/ZOLTAN --enable-mpi --enable-f90interface
$ make everything
$ make install
~~~


### TextParser

See Readme.md of TextParser



## EXAMPLES

* If you enable the compiling of test codes by setting `-Dwith_example=yes`, then you wull be able to execute the intrinsic tests by;

	`$ make test` or `$ ctest`

* The detailed results will be output at stdout and written as a file (LastTest.log) at the BUILD directory `/build/Testing/Temporary/`. 




## CONTRIBUTORS

* Kenji     Ono        _keno@cc.kyushu-u.ac.jp_, _keno@riken.jp_
* Yasuhiro  Kawashima
* Jorji     Nonaka     _jorji@riken.jp_
