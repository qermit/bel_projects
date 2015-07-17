#!/bin/bash
################################################################################

# Configuration
input_file=$1
eca_pattern=0xffff000000000000 # FID=MAX & GRPID=MAX
period=125000000
last=0
dev=0
ref_ip=0
ref_name=0
ref_io=0
ref_io_found=0

# Find reference device in list
echo "ECA-Multi-PPS script started ..."
[ ! -f $input_file ] && { echo "$input_file file not found"; exit 1; }
while read name ip io lenght
do
  # First line must contain the reference IO
  if [ $ref_io_found -ne 1 ]; then
    ref_io=$io
    ref_io_found=1
  fi
  if [ $io -eq $ref_io ]; then
    dev=$ip
    ref_ip=$ip
    ref_name=$name
  fi
done < $input_file

# Control ECA
while true; do
  # Try to schedule next pulse 3x a second.
  # This way we don't miss any pulses due to slow scheduling on linux.
  sleep 0.3
  # What time is it?
  when=`eca-ctl $dev -n | grep time | cut -d: -f2`
  # Round up to the next second.
  next="$(((when+period+period-1)/period*period))"
  # If we haven't scheduled this PPS pulse yet, do it now. 
  if [ $next -ne $last ]; then 
   next=$((next))
    [ ! -f $input_file ] && { echo "$input_file file not found"; exit 1; }
    while read name ip io lenght
    do
      # Don't drive IOs from reference device 
      if [ $io -ne $ref_io ]; then
        eca-ctl $ip send $eca_pattern 0 0 $next
        if [ $? -ne 0 ]; then
          echo "Warning: $name ($ip) at IO$io is not reachable!\n"
        fi
      fi
    done < $input_file
    time=$((next*8))
    printf "\rNext pulse at %d -> reference device %s (%s) ..." "$time" "$ref_name" "$ref_ip"
   next=$((next))
  fi
  last="$next"
done
