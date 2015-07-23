#!/bin/bash
################################################################################

# Configuration
input_file=$1
data_master=$2

# Run all scripts
./scripts/check-devices.sh $input_file
./scripts/configure-nodes.sh $input_file
./scripts/configure-data-master.sh $data_master
