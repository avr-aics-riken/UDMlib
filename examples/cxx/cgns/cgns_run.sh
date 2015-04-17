#!/bin/bash

# MPI_PATH=/usr/local/openmpi-1.6.5_gcc
# MPI_EXEC=$MPI_PATH/bin/mpiexec
MPI_EXEC=mpiexec
set -x

# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MPI_PATH/lib
# export PATH=$PATH:$MPI_PATH/bin

rm -rf model_4x4x4
rm -rf write_cgns_hexa_id000000
rm -rf conrod.tetra
rm -rf cgns_multi
rm -rf output

# create cgns
./create_cgns --name=model_4x4x4/cgns_hexa.cgns \
              --size=4,4,4 \
              --coords=1.0,2.0,3.0

# read cgns
./read_cgns model_4x4x4/cgns_hexa_id000000.cgns

# vtk2cgns file
$MPI_EXEC -np 4 ./vtk2cgns ../../data/torus.tetra.vtk

# create multi cgns
$MPI_EXEC -np 4 ./create_multi --name=cgns_multi/cgns_hexa.cgns \
                               --size=8,8,8

# user defined data
./user_defined model_4x4x4/index.dfi

