#!/bin/bash
################################################################################

# Configuration
input_file=$1
eca_pattern=0xC0CAC01A
period=125000000
last=0
dev=0
ref_ip=0
ref_name=0

# Find reference device in list
echo "ECA-Multi-PPS script started ..."
[ ! -f $input_file ] && { echo "$input_file file not found"; exit 1; }
while read name ip io lenght
do
  if [ $io -eq 0 ]; then
    dev=udp/$ip
    ref_ip=$ip
    ref_name=$name
  fi
done < $input_file

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
      # Don't drive IOs from reference device (io0)
      if [ $io -ne 0 ]; then
        eca-ctl udp/$ip send $eca_pattern 0 0 $next
        if [ $? -ne 0 ]; then
          echo "failed!"
          exit 1
        fi
      fi
    done < $input_file
    time=$((next*8))
    printf "\rNext pulse at %d -> reference device %s (%s) ..." "$time" "$ref_name" "$ref_ip"
   next=$((next))
  fi
  last="$next"
done
