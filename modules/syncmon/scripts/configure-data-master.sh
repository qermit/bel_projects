#!/bin/bash
################################################################################

# Variables
data_master=$1 # Data Master must be the first argument (dev/ttyUSBX, udp/192.168.0.1, ...)
data_master_bin=bin
eca_pattern=0xffff000000000000 # FID=MAX & GRPID=MAX
schedule=pps.xml
schedule_next=dm_pps.xml
schedule_keyword="___STARTTIME___"
start_offset=0x0000000030000000
start_time=0x0
period=125000000
wait_time=0

# Copy old schedule
cp "$schedule" "$schedule_next"

# Get time from ECA
time=`eca-ctl $data_master -n | grep time | cut -d: -f2`
time="$(($time+0))" # To dec
start_time="$(($time+$start_offset))" # Add offset
start_time="$(((start_time+period+period-1)/period*period))" # Round up to the next second

# Print debug infos
printf "Current time at Data Master: 0x%x (%d)\n" $time $time
printf "Start time at Data Master:   0x%x (%d)\n" $start_time $start_time

# Configure ECA (GPIO LEDs will indicate any data master activity)
eca-ctl $data_master enable # Enable
eca-ctl $data_master idisable # Disable interrupts
eca-table $data_master flush # Flush old stuff
# LEDs/channel0 (pulse width = 100ms)
eca-ctl $data_master activate -c 0
eca-table $data_master add $eca_pattern/0 +0.0 0 0x0000ffff
eca-table $data_master add $eca_pattern/0 +0.1 0 0xffff0000
# LEMOs/channel2 (pulse width = 100ms))
eca-ctl $data_master activate -c 2
eca-table $data_master add $eca_pattern/64 +0.0 2 0x000ffe
eca-table $data_master add $eca_pattern/64 +0.1 2 0xffe000
eca-table $data_master flip-active

# Get right start time in the schedule
sed -i "s/$schedule_keyword/$start_time/g" "$schedule_next"

# Finally set up the Data Master
mv $schedule_next $data_master_bin
cd $data_master_bin
#./ftm-ctl $data_master -c -1 loadfw ftm.bin
#sleep 1
./ftm-ctl $data_master -c 0 put $schedule_next
sleep 1
./ftm-ctl $data_master -c 0 swap 
sleep 1
./ftm-ctl $data_master -c 0 run
sleep 1
cd ..

# Wait until Data Master should start
while [ $start_time -ge  $time ]; do
  wait_time="$(($start_time-$time))"
  printf "\rData Master will start in %dns..." "$wait_time"
  time=`eca-ctl $data_master -n | grep time | cut -d: -f2`
  time="$(($time+0))" # To dec
  #printf "Current time at Data Master: 0x%x (%d)\n" $time $time
  #printf "Start time at Data Master:   0x%x (%d)\n" $start_time $start_time
done

printf "\rData Master will start in 0ns...                    \n" 
echo "Data Master started!"
