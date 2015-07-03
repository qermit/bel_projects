#!/bin/bash
################################################################################

# Configuration
input_file=$1
eca_pattern=0xC0CAC01A
iodir_lvds_oe_addr=0x10
iodir_lemo_oe_addr=0x14
io_oe_pattern=0xffffffff

# Function configure_iodir
# -- Used to enable every output on a device
# $1 = IP of target device
function configure_iodir()
{
  echo -n "Configuring output enable at $1 ($2) ... "
  eb-write udp/$2 $iodir_lvds_oe_addr/4 $io_oe_pattern
  if [ $? -ne 0 ]; then
   echo "failed!"
   exit 1
  fi
  eb-write udp/$2 $iodir_lemo_oe_addr/4 $io_oe_pattern
  if [ $? -ne 0 ]; then
   echo "failed!"
   exit 1
  fi
  echo "done."
}

# Function configure_eca
# -- Used to configure a ECA table to the given pattern
# $1 = IP of target device
# $2 = ECA pattern
function configure_eca()
{
  echo -n "Configuring ECA at $1 ($2) ... "
  # Enable
  eca-ctl udp/$2 enable
  # Disable interrupts
  eca-ctl udp/$2 idisable 
  # Flush old stuff
  eca-table udp/$2 flush
  # LEDs/ channel0 (pulse width = 100ms)
  eca-ctl udp/$2 activate -c 0
  eca-table udp/$2 add $eca_pattern/64 +0.0 0 0x0000ffff
  eca-table udp/$2 add $eca_pattern/64 +0.1 0 0xffff0000
  # LEMOs/channel2 (pulse width = 204*8=1632ns ~= Switch PPS)
  eca-ctl udp/$2 activate -c 2
  eca-table udp/$2 add $eca_pattern/64 0 2 0x000fff
  eca-table udp/$2 add $eca_pattern/64 204 2 0xfff000
  eca-table udp/$2 flip-active
  echo "done."
}

# Script starts here
echo "Configuration script started ..."
[ ! -f $input_file ] && { echo "$input_file file not found"; exit 1; }
while read name ip io lenght
do
  # Don't configure reference device (io0)
  if [ $io -ne 0 ]; then
    configure_iodir $name $ip
    configure_eca   $name $ip
  fi
done < $input_file

# Delete old log files
echo "Deleting old log files ..."
rm -r log
mkdir log
touch log/dummy
