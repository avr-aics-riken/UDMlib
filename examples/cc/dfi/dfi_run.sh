#!/bin/bash

# MPI_PATH=/usr/local/openmpi-1.6.5_gcc
# MPI_EXEC=$MPI_PATH/bin/mpiexec
MPI_EXEC=mpiexec
set -x

# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MPI_PATH/lib
# export PATH=$PATH:$MPI_PATH/bin

rm -rf model_4x4x4
rm -rf output

# create cgns
../cgns/create_cgns --name=model_4x4x4/cgns_hexa.cgns \
              --size=4,4,4

# unit list
./dfi_unit model_4x4x4/index.dfi


