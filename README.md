# UDMlib - Unstructured Data Management library

# Copyright (c) 2012-2015 Institute of Industrial Science, The University of Tokyo. All rights reserved.
# Copyright (c) 2015-2017 Advanced Institute for Computational Science, RIKEN. All rights reserved.
# Copyright (c) 2017 Research Institute for Information Technology (RIIT), Kyushu University. All rights reserved.


## OUTLINE

UDMlib is a file management library for unstructured dataset, and provides following functions:

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

>  Specify the directory that this library will be installed. Built library is installed at `install_directory/lib` and the header files are placed at `install_directory/include`. The default install directory is `/usr/local/UDMlib`.

`-D with_util=` {yes | no}

>  Install utility tools. The default of this option is yes.
>
>  In case of cross-compilation, the frm tool is not installed. On the other hand, in case of native compile environment, `-D with_util=yes` indicates that all utilities are installed at the same time as CDMlib is compiled.
>
>  If `with_NetCDF=no` is specified, NetCDF function will not be supported.
>
>  If `with_MPI=no`, fconv and upacs2dfi will not be built.
>  

`-D with_MPI=` {yes | no}

>  If you use an MPI library, specify `with_MPI=yes`, the default is yes.

`-D with_TP =` *TextParser_directory*

> Specify the directory path that TextParser is installed.

`-D with_CGNS=` *CGNS_directory*

> Specify the directory path that CGNS is installed.

`-D with_HDF5=` *HDF5_directory*

> Specify the directory path that HDF5 is installed.

`-D with_ZOLTAN=` *Zoltan_directory*

> Specify the directory path that Zoltan library is installed. Zoltan library must be the same compiler with the one used to build OpenMPI library.

`-D with_example=` {no | yes}

>  This option turns on compiling sample codes. The default is no.


The default compiler options are described in `cmake/CompilerOptionSelector.cmake` file. See BUILD OPTION section in CMakeLists.txt in detail.


## Configure Examples

`$ export UDM_HOME=hogehoge`

In following examples, assuming that TextParser, CGNS, and Zoltan libraries are installed under the UDM_HOME directory. If not, please specify applicable directory path.


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
$ cmake -DINSTALL_DIR=${CDM_HOME}/CDMlib \
            -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain_fx10.cmake \
            -Dwith_MPI=yes \
            -Dwith_example=no \
            -Dwith_util=yes \
            -Dwith_TP=${CDM_HOME}/TextParser \
            -Dwith_CPM=${CDM_HOME}/CPMlib \
            -Dwith_HDF=no \
            -Dwith_NetCDF=no \
            -Denable_BUFFER_SIZE=no ..

$ cmake -DINSTALL_DIR=${CDM_HOME}/CDMlib \
            -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain_fx100.cmake \
            -Dwith_MPI=yes \
            -Dwith_example=no \
            -Dwith_util=yes \
            -Dwith_TP=${CDM_HOME}/TextParser \
            -Dwith_CPM=${CDM_HOME}/CPMlib \
            -Dwith_HDF=no \
            -Dwith_NetCDF=no \
            -Denable_BUFFER_SIZE=no ..

$ cmake -DINSTALL_DIR=${CDM_HOME}/CDMlib \
            -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain_K.cmake \
            -Dwith_MPI=yes \
            -Dwith_example=no \
            -Dwith_util=yes \
            -Dwith_TP=${CDM_HOME}/TextParser \
            -Dwith_CPM=${CDM_HOME}/CPMlib \
            -Dwith_HDF=no \
            -Dwith_NetCDF=no \
            -Denable_BUFFER_SIZE=no ..
~~~


##### Note
- On Fujitsu machines(fx10, K, fx100), confirm appropriate directrory path for compiler environment.
- Before building, execute following command for clean. `$ make distclean`



### Compilation of staging tool(frm)

When you install staging tool onto a login node with cross-compiling environment, the tool must be compiled by a native GNU compiler on the login node. If the front-end login node does not have MPI library, specify `-Dwith_MPI=no` option.



## Note for building required libraries

### OpenMPI

~~~
$ cat config_ompi.sh
#!/bin/sh
export CC=icc
export CFLAGS=-O3
export CXX=icpc
export CXXFLAGS=-O3
export F77=ifort
export FFLAGS=-O3
export FC=ifort
export FCFLAGS=-O3
./configure --prefix=$1
~~~

~~~
$ config_ompi.sh /install-dir
$ make && make install
~~~


### Zlib

~~~
$ tar xvfz zlib-x.x.x.tar.gz
$ cd zlib-x.x.x
$ export CC=icc && ./configure --prefix=${HOME}/Zlib
$ make && make install
~~~


### SZIP

~~~
$ tar xvfz szip-x.x.x.tar.gz
$ cd szip-x.x.x
$ export CC=icc CFLAGS=-O3
$ export CXX=icpc CXXFLAGS=-O3
$ ./configure --prefix=${HOME}/SZIP
$ make && make install
~~~

### HDF5

~~~
$ tar xvfz hdf5-x.x.x.tar.gz
$ cd hdf5-x.x.x
$ export CC=icc CFLAGS=-O3
$ export CXX=icpc CXXFLAGS=-O3
$ export FC=ifort FCFLAGS=-O3
$ ./configure --prefix=${HOME}/HDF5 --enable-64bit \
              --enable-fortran \
              --with-zlib=${HOME}/Zlib/include,${HOME}/Zlib/lib \
              --enable-cxx \
              --with-szip=${HOME}/SZIP
$ make && make install
~~~


### CGNS

-  Before building cgns library, you need to move header file zlib.h into the directory where libz.a exist (lib/ directory).

~~~
$ tar xzvf cgnslib_x.x.x.tar.gz
$ cd cgnslib_x.x.x/src
$ export CC=mpicc CFLAGS=-O3
$ export FC=mpif90 FCFLAGS=-O3
$ export HDF5_LIBRARY_DIR=${HOME}/ompi-intel/HDF5/lib
$ export HDF5_INCLUDE_DIR=${HOME}/ompi-intel/HDF5/include
$ ./configure --prefix=$1 \
              --enable-64bit \
              --with-fortran \
              --with-hdf5=${HOME}/HDF5 \
              --with-szip=${HOME}/SZIP/lib/libsz.a \
              --with-zlib=${HOME}/Zlib/lib/libz.a
$ make && make install
~~~


### Zoltan

~~~
$ tar xvfz zoltan_distrib_v3.81.tar.gz
$ cd Zoltan_v3.81
$ mkdir BUILD_DIR
$ cd BUILD_DIR
$ export CC=mpicc CFLAGS=-O3 FC=mpif90 FCFLAGS=-O3
$ ../configure --prefix=${HOME}/ZOLTAN --enable-mpi --enable-f90interface
$ make everything
$ make install
~~~


### TextParser

See Readme.md of TextParser


## EXAMPLES

* If you specify the test option by `-Denable_example=yes`, you can
execute the intrinsic tests by;

	`$ make test` or `$ ctest`

* The detailed results are written in `BUILD/Testing/Temporary/LastTest.log` file.
Meanwhile, the summary is displayed for stdout.




## CONTRIBUTORS

* Kenji     Ono        _keno@cc.kyushu-u.ac.jp_, _keno@riken.jp_
* Yasuhiro  Kawashima
* Jorji     Nonaka     _jorji@riken.jp_
