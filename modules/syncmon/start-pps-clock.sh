#!/bin/bash
################################################################################

# Configuration
input_file=$1
ref_io=0
ref_io_found=0
ref_name=""

# Script starts here
echo "Configuration script started ..."
[ ! -f $input_file ] && { echo "$input_file file not found"; exit 1; }
while read name ip io lenght
do
  # First line must contain the reference IO
  if [ $ref_io_found -ne 1 ]; then
    ref_io=$io
    ref_io_found=1
    ref_name=$name
  fi
  # Don't configure reference device (io0)
  if [ $io -ne $ref_io ]; then
    cd bin
    echo -n "Setting clock for $name at $ip ... "
    ./eb-clock -c 1 -H 500000000 -L 500000000 $ip > /dev/null 2>&1
    if [ $? -ne 0 ]; then
      echo "failed!"
    else
      echo "okay."
    fi
    sleep 0.5
    cd ..
  fi
done < $input_file

# Delete old log files
echo "Deleting old log files ..."
cd log
rm $ref_name*
cd ..
