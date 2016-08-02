#!/bin/bash
################################################################################

# Configuration
input_file=$1
ref_io=0
ref_io_found=0
ref_name=""

# Search for reference name
[ ! -f $input_file ] && { echo "$input_file file not found"; exit 1; }
while read name ip io lenght
do
  # First line must contain the reference IO
  if [ $ref_io_found -ne 1 ]; then
    ref_io=$io
    ref_io_found=1
    ref_name=$name
  fi
done < $input_file

# Delete old log files
echo "Deleting old log files ..."
cd log
rm $ref_name*
cd ..

# Start syncmon
./syncmon $1
