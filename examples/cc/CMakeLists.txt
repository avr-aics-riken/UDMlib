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


include_directories(
      ${PROJECT_BINARY_DIR}/include
      ${PROJECT_SOURCE_DIR}/include
      ${ZOLTAN_INC}
      ${TP_INC}
      ${HDF5_INC}
      ${CGNS_INC}
)

link_directories(
      ${PROJECT_BINARY_DIR}/src
      ${ZOLTAN_LIB}
      ${CGNS_LIB}
      ${HDF5_LIB}
      ${TP_LIB}
)


#########################
# cgns
#########################

#### Test1 : create_cgns

add_executable(create_cgns_c cgns/create_cgns.c)
target_link_libraries(create_cgns_c -lUDMmpi -lTPmpi -lzoltan -lcgns -lhdf5)
set (test_parameters
    "--name=model_4x4x4/cgns_hexa.cgns"
    "--size=4,4,4"
    "--coords=1.0,2.0,3.0"
    )
add_test(NAME TEST_C_1 COMMAND "./create_cgns_c" ${test_parameters})


### Test2 : read_cgns

add_executable(read_cgns_c cgns/read_cgns.c)
target_link_libraries(read_cgns_c -lUDMmpi -lTPmpi -lzoltan -lcgns -lhdf5)
set (test_parameters
    "model_4x4x4/cgns_hexa_id000000.cgns"
    )
add_test(NAME TEST_C_2 COMMAND "read_cgns_c" ${test_parameters})


### Test3 : vtk2cgns

add_executable(vtk2cgns_c cgns/vtk2cgns.c)
target_link_libraries(vtk2cgns_c -lUDMmpi -lTPmpi -lzoltan -lcgns -lhdf5)
set (test_parameters -np 4
    "./vtk2cgns_c"
    "${PROJECT_SOURCE_DIR}/examples/data/torus.tetra.vtk"
    )
add_test(NAME TEST_C_3 COMMAND "mpirun" ${test_parameters})


### Test4 : create_multi

add_executable(create_multi_c cgns/create_multi.c)
target_link_libraries(create_multi_c -lUDMmpi -lTPmpi -lzoltan -lcgns -lhdf5)
set (test_parameters -np 4
    "./create_multi_c"
    "--name=cgns_multi/cgns_hexa.cgns"
    "--size=8,8,8"
    "--coords=1.0,2.0,3.0"
    )
add_test(NAME TEST_C_4 COMMAND "mpirun" ${test_parameters})


### Test5 : user_defined

add_executable(user_defined_c cgns/user_defined.c)
target_link_libraries(user_defined_c -lUDMmpi -lTPmpi -lzoltan -lcgns -lhdf5)
set (test_parameters
    "model_4x4x4/index.dfi"
    )
add_test(NAME TEST_C_5 COMMAND "user_defined_c" ${test_parameters})



#########################
# dfi
#########################

### Test6 : dfi_unit

add_executable(dfi_unit_c dfi/dfi_unit.c)
target_link_libraries(dfi_unit_c -lUDMmpi -lTPmpi -lzoltan -lcgns -lhdf5)
set (test_parameters
    "model_4x4x4/index.dfi"
    )
add_test(NAME TEST_C_6 COMMAND "dfi_unit_c" ${test_parameters})



#########################
# partition
#########################

#### Test7 : create_cgns

set (test_parameters
    "--name=input_32/cgns_hexa.cgns"
    "--size=32,32,32"
    "--coords=1.0,1.0,1.0"
    )
add_test(NAME TEST_C_7 COMMAND "./create_cgns_c" ${test_parameters})


### Test8 : partition : partition from 1 to 4

add_executable(partition_c partition/partition.c)
target_link_libraries(partition_c -lUDMmpi -lTPmpi -lzoltan -lcgns -lhdf5)
set (test_parameters -np 4
    "./partition_c"
    "input_32/index.dfi"
    "--output=../output_32-m1-p4"
    )
add_test(NAME TEST_C_8 COMMAND "mpirun" ${test_parameters})


### Test9 : partition : partition from 4 to 8

set (test_parameters -np 8
    "./partition_c"
    "output_32-m1-p4/index.dfi"
    "--output=../output_32-m4-p8"
    )
add_test(NAME TEST_C_9 COMMAND "mpirun" ${test_parameters})


### Test10 : partition : partition from 8 to 16

set (test_parameters -np 16
    "./partition_c"
    "output_32-m4-p8/index.dfi"
    "--output=../output_32-m8-p16"
    )
add_test(NAME TEST_C_10 COMMAND "mpirun" ${test_parameters})


### Test11 : partition : partition from 16 to 8

set (test_parameters -np 8
    "./partition_c"
    "output_32-m8-p16/index.dfi"
    "--output=../output_32-m16-p8"
    )
add_test(NAME TEST_C_11 COMMAND "mpirun" ${test_parameters})


### Test12 : partition : partition from 8 to 4

set (test_parameters -np 4
    "./partition_c"
    "output_32-m16-p8/index.dfi"
    "--output=../output_32-m8-p4"
    )
add_test(NAME TEST_C_12 COMMAND "mpirun" ${test_parameters})


### Test13 : partition : partition from 4 to 1

set (test_parameters -np 4
    "./partition_c"
    "output_32-m8-p4/index.dfi"
    "--output=../output_32-m4-p1"
    )
add_test(NAME TEST_C_13 COMMAND "mpirun" ${test_parameters})


### Test14 : partition_weight

add_executable(partition_weight_c partition/partition_weight.c)
target_link_libraries(partition_weight_c -lUDMmpi -lTPmpi -lzoltan -lcgns -lhdf5)
set (test_parameters -np 8
    "./partition_weight_c"
    "input_32/index.dfi"
    )
add_test(NAME TEST_C_14 COMMAND "mpirun" ${test_parameters})


### Test15 : partition_solutions

add_executable(partition_solutions_c partition/partition_solutions.c)
target_link_libraries(partition_solutions_c -lUDMmpi -lTPmpi -lzoltan -lcgns -lhdf5)
set (test_parameters -np 8
    "./partition_solutions_c"
    "input_32/index.dfi"
    )
add_test(NAME TEST_C_15 COMMAND "mpirun" ${test_parameters})



#########################
# timeslice
#########################

#### Test16 : create_cgns

set (test_parameters
    "--name=input/cgns_hexa.cgns"
    "--size=8,8,8"
    "--coords=1.0,1.0,1.0"
    )
add_test(NAME TEST_C_16 COMMAND "./create_cgns_c" ${test_parameters})


#### Test17-27 : create_timeslice

add_executable(create_timeslice_c timeslice/create_timeslice.c)
target_link_libraries(create_timeslice_c -lUDMmpi -lTPmpi -lzoltan -lcgns -lhdf5)
set (test_parameters
    "--input=input/index.dfi"
    "--output=includegrid_eachstep_gridconstant"
    "--includegrid"
    "--eachstep"
    "--gridconstant"
    )
add_test(NAME TEST_C_17 COMMAND "./create_timeslice_c" ${test_parameters})

set (test_parameters
    "--input=input/index.dfi"
    "--output=includegrid_eachstep_gridtimeslice"
    "--includegrid"
    "--eachstep"
    "--gridtimeslice"
    )
add_test(NAME TEST_C_18 COMMAND "./create_timeslice_c" ${test_parameters})

set (test_parameters
    "--input=input/index.dfi"
    "--output=includegrid_appendstep_gridconstant"
    "--includegrid"
    "--appendstep"
    "--gridconstant"
    )
add_test(NAME TEST_C_19 COMMAND "./create_timeslice_c" ${test_parameters})

set (test_parameters
    "--input=input/index.dfi"
    "--output=includegrid_appendstep_gridtimeslice"
    "--includegrid"
    "--appendstep"
    "--gridtimeslice"
    )
add_test(NAME TEST_C_20 COMMAND "./create_timeslice_c" ${test_parameters})

set (test_parameters
    "--input=input/index.dfi"
    "--output=excludegrid_eachstep_gridconstant"
    "--excludegrid"
    "--eachstep"
    "--gridconstant"
    )
add_test(NAME TEST_C_21 COMMAND "./create_timeslice_c" ${test_parameters})

set (test_parameters
    "--input=input/index.dfi"
    "--output=excludegrid_eachstep_gridtimeslice"
    "--excludegrid"
    "--eachstep"
    "--gridtimeslice"
    )
add_test(NAME TEST_C_22 COMMAND "./create_timeslice_c" ${test_parameters})

set (test_parameters
    "--input=input/index.dfi"
    "--output=excludegrid_appendstep_gridconstant"
    "--excludegrid"
    "--appendstep"
    "--gridconstant"
    )
add_test(NAME TEST_C_23 COMMAND "./create_timeslice_c" ${test_parameters})

set (test_parameters
    "--input=input/index.dfi"
    "--output=excludegrid_appendstep_gridtimeslice"
    "--excludegrid"
    "--appendstep"
    "--gridtimeslice"
    )
add_test(NAME TEST_C_24 COMMAND "./create_timeslice_c" ${test_parameters})

set (test_parameters
    "--input=input/index.dfi"
    "--output=field_directory"
    "--excludegrid"
    "--eachstepp"
    "--gridtimeslice"
    "--with_directorypath=field_data"
    )
add_test(NAME TEST_C_25 COMMAND "./create_timeslice_c" ${test_parameters})

set (test_parameters
    "--input=input/index.dfi"
    "--output=timeslice_directory"
    "--excludegrid"
    "--eachstep"
    "--gridtimeslice"
    "--with_timeslice_directory"
    )
add_test(NAME TEST_C_26 COMMAND "./create_timeslice_c" ${test_parameters})

set (test_parameters
    "--input=input/index.dfi"
    "--output=field_timeslice"
    "--excludegrid"
    "--eachstep"
    "--gridtimeslice"
    "--with_directorypath=field_data"
    "--with_timeslice_directory"
    )
add_test(NAME TEST_C_27 COMMAND "./create_timeslice_c" ${test_parameters})


#### Test28-40 : timeslice

add_executable(timeslice_c timeslice/timeslice.c)
target_link_libraries(timeslice_c -lUDMmpi -lTPmpi -lzoltan -lcgns -lhdf5)
set (test_parameters  -np 4
    "./timeslice_c"
    "./includegrid_eachstep_gridconstant/index.dfi"
    )
add_test(NAME TEST_C_28 COMMAND "mpirun" ${test_parameters})
