#!/bin/bash
################################################################################

# Configuration
input_file=$1
data_master=$2

# Run all scripts
./scripts/check-devices.sh $input_file
./scripts/configure.sh $input_file
./scripts/configure_data_master.sh $data_master
