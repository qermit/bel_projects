#!/bin/bash
################################################################################

# Configuration
input_file=$1
eca_pattern=0xffff000000000000 # FID=MAX & GRPID=MAX
ref_io=0
ref_io_found=0
ref_name=""

# Function configure_iodir
# -- Used to enable every output on a device
# $1 = IP of target device
function configure_iodir()
{
  ./oe-config $ip
}

# Function configure_eca
# -- Used to configure a ECA table to the given pattern
# $1 = IP of target device
# $2 = ECA pattern
function configure_eca()
{
  echo -n "Configuring ECA at $1 ($2) ... "
  # Enable
  eca-ctl $2 enable
  # Disable interrupts
  eca-ctl $2 idisable 
  # Flush old stuff
  eca-table $2 flush
  # LEDs/ channel0 (pulse width = 100ms)
  eca-ctl $2 activate -c 0
  eca-table $2 add $eca_pattern/64 +0.0 0 0x0000ffff
  eca-table $2 add $eca_pattern/64 +0.1 0 0xffff0000
  # LEMOs/channel2 (pulse width = 100ms))
  eca-ctl $2 activate -c 2
  eca-table $2 add $eca_pattern/64 +0.0 2 0x000fff
  eca-table $2 add $eca_pattern/64 +0.1 2 0xfff000
  eca-table $2 flip-active
  echo "done."
}

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
    configure_iodir $name $ip
    configure_eca   $name $ip
  fi
  sleep 0.5
done < $input_file

# Delete old log files
echo "Deleting old log files ..."
cd log
rm $ref_name*
cd ..
