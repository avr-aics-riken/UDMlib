#!/bin/bash

TOP_DIR=`pwd`
cd $TOP_DIR/cgns
sh ./cgns_run.sh
if [ $? -ne 0 ]; then exit;fi

cd $TOP_DIR/partition
sh ./partition_run.sh
if [ $? -ne 0 ]; then exit;fi

cd $TOP_DIR/timeslice
sh ./timeslice_run.sh
if [ $? -ne 0 ]; then exit;fi

cd $TOP_DIR/dfi
sh ./dfi_run.sh
if [ $? -ne 0 ]; then exit;fi

