#!/bin/bash

PROGRAM=./partition
PROGRAM_WEIGHT=./partition_weight
PROGRAM_SOLUTION=./partition_solutions
# MPI_PATH=/usr/local/openmpi-1.6.5_gcc
# MPI_EXEC=$MPI_PATH/bin/mpiexec
MPI_EXEC=mpiexec
MODEL_SIZE=32
set -x

# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MPI_PATH/lib
rm -rf input_$MODEL_SIZE
rm -rf output
rm -rf output_$MODEL_SIZE*
rm -rf output_weight
rm -rf output_noneweight
rm -rf output_solution

# create model
../cgns/create_cgns --name=input_$MODEL_SIZE/cgns_hexa.cgns \
                    --size=$MODEL_SIZE,$MODEL_SIZE,$MODEL_SIZE \
                    --coords=1.0,1.0,1.0

# partition from 1 to 4
$MPI_EXEC -np 4 $PROGRAM  input_$MODEL_SIZE/index.dfi
if [ $? -ne 0 ]; then exit;fi
mv output output_$MODEL_SIZE-m1-p4

# partition from 4 to 8
$MPI_EXEC -np 8 $PROGRAM  output_$MODEL_SIZE-m1-p4/index.dfi
if [ $? -ne 0 ]; then exit;fi
mv output output_$MODEL_SIZE-m4-p8

# partition from 8 to 16
$MPI_EXEC -np 16 $PROGRAM  output_$MODEL_SIZE-m4-p8/index.dfi
if [ $? -ne 0 ]; then exit;fi
mv output output_$MODEL_SIZE-m8-p16

# partition from 16 to 8
$MPI_EXEC -np 8 $PROGRAM  output_$MODEL_SIZE-m8-p16/index.dfi
if [ $? -ne 0 ]; then exit;fi
mv output output_$MODEL_SIZE-m16-p8

# partition from 8 to 4
$MPI_EXEC -np 4 $PROGRAM  output_$MODEL_SIZE-m16-p8/index.dfi
if [ $? -ne 0 ]; then exit;fi
mv output output_$MODEL_SIZE-m8-p4

# partition from 4 to 1
$MPI_EXEC -np 1 $PROGRAM  output_$MODEL_SIZE-m8-p4/index.dfi
if [ $? -ne 0 ]; then exit;fi
mv output output_$MODEL_SIZE-m4-p1

# partition with weight
$MPI_EXEC -np 8 $PROGRAM_WEIGHT input_$MODEL_SIZE/index.dfi

# partition with solution
$MPI_EXEC -np 8 $PROGRAM_SOLUTION input_$MODEL_SIZE/index.dfi


