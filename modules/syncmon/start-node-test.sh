#!/bin/bash
################################################################################

# Configuration
input_file=$1

# Run all scripts
./scripts/check-devices.sh $input_file
./scripts/configure-nodes.sh $input_file
