#!/bin/bash
################################################################################

# Configuration
input_file=$1

# Delete old log files
echo "Deleting old log files ..."
cd log
rm $ref_name*
cd ..

# Start syncmon
./syncmon $1
