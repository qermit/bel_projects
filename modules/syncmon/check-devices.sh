#!/bin/bash
################################################################################

# Configuration
input_file=$1

# Check each device
echo "Device check script started ..."
while read name ip io lenght
do
  echo -n "Checking device $name ($ip) ... "
  eb-ls $ip >/dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo "failed! ($name ($ip) is not reachable!)"
  else
    echo "okay."
  fi
done < $input_file
