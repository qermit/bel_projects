#!/bin/bash
################################################################################

# Configuration
input_file=devices.cfg
eca_pattern=0xC0CAC01A
dev="$1"
period=125000000
last=0

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
    while read name ip io
    do
      eca-ctl udp/$ip send $eca_pattern 0 0 $next
    done < $input_file
    time=$((next*8))
    printf "\rNext pulse at %d ..." "$time"
   next=$((next))
  fi
  last="$next"
done
