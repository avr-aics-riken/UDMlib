#!/bin/bash

# MPI_PATH=/usr/local/openmpi-1.6.5_gcc
# MPI_EXEC=$MPI_PATH/bin/mpiexec
MPI_EXEC=mpiexec
set -x

# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MPI_PATH/lib
# export PATH=$PATH:$MPI_PATH/bin

rm -rf input
rm -rf output
rm -rf includegrid_*
rm -rf excludegrid_*
rm -rf field_directory
rm -rf timeslice_directory
rm -rf field_timeslice

# create cgns
../cgns/create_cgns --name=input/cgns_hexa.cgns \
                    --size=8,8,8

# create timeslice dfi
./create_timeslice --input=input/index.dfi --output=includegrid_eachstep_gridconstant --includegrid  --eachstep --gridconstant
./create_timeslice --input=input/index.dfi --output=includegrid_eachstep_gridtimeslice  --includegrid  --eachstep --gridtimeslice
./create_timeslice --input=input/index.dfi --output=includegrid_appendstep_gridconstant  --includegrid  --appendstep --gridconstant
./create_timeslice --input=input/index.dfi --output=includegrid_appendstep_gridtimeslice  --includegrid  --appendstep --gridtimeslice
./create_timeslice --input=input/index.dfi --output=excludegrid_eachstep_gridconstant  --excludegrid  --eachstep --gridconstant
./create_timeslice --input=input/index.dfi --output=excludegrid_eachstep_gridtimeslice  --excludegrid  --eachstep --gridtimeslice
./create_timeslice --input=input/index.dfi --output=excludegrid_appendstep_gridconstant  --excludegrid  --appendstep --gridconstant
./create_timeslice --input=input/index.dfi --output=excludegrid_appendstep_gridtimeslice  --excludegrid  --appendstep --gridtimeslice
./create_timeslice --input=input/index.dfi --output=field_directory  --excludegrid  --eachstep --gridtimeslice --with_directorypath=field_data
./create_timeslice --input=input/index.dfi --output=timeslice_directory  --excludegrid  --eachstep --gridtimeslice --with_timeslice_directory
./create_timeslice --input=input/index.dfi --output=field_timeslice  --excludegrid  --eachstep --gridtimeslice --with_directorypath=field_data --with_timeslice_directory

# timeslice
$MPI_EXEC -np 4 ./timeslice ./includegrid_eachstep_gridconstant/index.dfi
$MPI_EXEC -np 4 ./timeslice ./includegrid_eachstep_gridtimeslice/index.dfi
$MPI_EXEC -np 4 ./timeslice ./includegrid_appendstep_gridconstant/index.dfi
$MPI_EXEC -np 4 ./timeslice ./includegrid_appendstep_gridtimeslice/index.dfi
$MPI_EXEC -np 4 ./timeslice ./includegrid_appendstep_gridtimeslice/index.dfi --output=output_without_cell --without_cell
$MPI_EXEC -np 4 ./timeslice ./includegrid_appendstep_gridtimeslice/index.dfi --output=output_without_node --without_node
$MPI_EXEC -np 4 ./timeslice ./excludegrid_eachstep_gridconstant/index.dfi
$MPI_EXEC -np 4 ./timeslice ./excludegrid_eachstep_gridtimeslice/index.dfi
$MPI_EXEC -np 4 ./timeslice ./excludegrid_appendstep_gridconstant/index.dfi
$MPI_EXEC -np 4 ./timeslice ./excludegrid_appendstep_gridtimeslice/index.dfi
$MPI_EXEC -np 4 ./timeslice ./field_directory/index.dfi
$MPI_EXEC -np 4 ./timeslice ./timeslice_directory/index.dfi
$MPI_EXEC -np 4 ./timeslice ./field_timeslice/index.dfi

