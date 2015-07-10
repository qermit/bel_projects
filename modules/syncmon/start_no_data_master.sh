#!/bin/bash
################################################################################

# Configuration
input_file=$1

# Run all scripts
./check-devices.sh $input_file
./configure.sh $input_file
./eca-multi-pps.sh $input_file

