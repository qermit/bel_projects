#!/usr/bin/env python
import numpy as np
import matplotlib.pyplot as plt
import subprocess
import sys, getopt
import os.path
import time

# Function plot(...)
# --------------------------------------------------------------------------------
def plot():
  # Reset min and max plot limits
  min_val_plot = -200
  max_val_plot = 200

  # Read configuration file (get name and io number)
  data_ref = np.genfromtxt(filename,delimiter=' ', dtype=np.str)
  device_names = [row[0] for row in data_ref]
  data_ref = np.genfromtxt(filename,delimiter=' ', dtype=np.uint64)
  device_ios = [row[2] for row in data_ref]
  
  # Check every device in list now
  f, (ax1, ax2) = plt.subplots(2, 1)
  index = 0
  
  for i in device_ios:
    help_buffer = "log/%s_syncmon_dev_plot_io%d.log" % (device_names[0], i)
    dev_name_buffer = "%s (reference 0.0ns)" % str(device_names[index])
    print "Reading log file: %s" % help_buffer
    
    # Check if the log file exists
    if os.path.isfile(help_buffer):
      # Reference device
      if i == device_ios[0]:
        # Get data from file
        data_ref = np.genfromtxt(help_buffer,delimiter=' ', dtype=np.int64)
        time_ref = [row[0] for row in data_ref]
        value_ref = [row[1] for row in data_ref]
        value_refz = range(len(value_ref))
        # Set value(s) to zero, we only care about the time difference (not the timestamp)
        for i in range (0, len(value_refz)):
          value_refz[i] = 0;
        ax1.plot(time_ref,value_refz,lw=2, linestyle='-', label=dev_name_buffer)
        ax2.plot(time_ref,value_refz,lw=2, linestyle='-', label=dev_name_buffer)
        
      # Device under test
      else:
        # Get data from file
        data_dev = np.genfromtxt(help_buffer,delimiter=' ', dtype=np.int64)
        time_dev = [row[0] for row in data_dev]
        value_dev = [row[1] for row in data_dev]
        average_dev = 0.0
        
        # Get the max possible lenght of the plot
        ref_elements = len(value_ref)
        dev_elements = len(value_dev)
        if ref_elements < dev_elements:
          min_elements = ref_elements
        else:
          min_elements = dev_elements
        
        # Calculate difference be reference and device under test
        for i in range (0, min_elements):
          value_dev[i] = np.int64(value_dev[i]) - np.int64(value_ref[i])
          # Calculate average
          average_dev = average_dev + value_dev[i]
          # Get min and max value for plotting
          if min_val_plot > value_dev[i]:
             min_val_plot = value_dev[i]
	  if max_val_plot < value_dev[i]:
             max_val_plot = value_dev[i]
        
        # Create legend with average note and plot it
        average_dev = average_dev/len(value_dev)
        dev_name_buffer = "%s (%fns)" % (str(device_names[index]), average_dev)
        ax1.plot(time_dev,value_dev,lw=2, linestyle='-', label=dev_name_buffer)
        ax2.plot(time_dev,value_dev,lw=2, linestyle='-', label=dev_name_buffer)
    
    # If the file does not exist
    else:
      print "Log file %s does not exist!" % help_buffer
    
    # Go for next device in list
    index = index + 1
      
  # Plot settings (-200ns to 200ns)
  ax1.set_title('Synchronization Monitor for White Rabbit (-200ns to 200ns range)')
  ax1.set_ylabel('Time Difference [ns]')
  ax1.set_xlabel('PPS Count')
  ax1.set_xlim([1,(len(value_ref)-1)])
  ax1.set_ylim([-200,200])
  ax1.grid()
  ax1.legend(loc=2)
  
  # Plot settings (auto scale)
  ax2.set_title('Synchronization Monitor for White Rabbit (auto scale, gray span shows -200ns to 200ns range)')
  ax2.set_ylabel('Time Difference [ns]')
  ax2.set_xlabel('PPS Count')
  ax2.set_xlim([1,(len(value_ref)-1)])
  ax2.set_ylim([min_val_plot,max_val_plot])
  ax2.axhspan(-200, 200, facecolor='0.25', alpha=0.25)
  ax2.grid()
  #ax2.legend(loc=2)
  
  # Show plot
  #mng = plt.get_current_fig_manager()
  #mng.frame.Maximize(True)
  mng = plt.get_current_fig_manager()
  mng.resize(*mng.window.maxsize())

# Function main(...)
# --------------------------------------------------------------------------------
def main(argv):
  # Helpers
  global device_count # Total devices to plot
  global device_names # List of all device names
  global device_ios   # List of all device ios
  global device_pps_total
  global device_pps_ids
  global device_time_stamps
  global deivce_time_diffs
  global value_refz
  global filename
  global update_rate
  global min_val_plot
  global max_val_plot
  
  # Check if a file name was given as argument
  if (len(sys.argv) == 3):
    filename = sys.argv[1]
    update_rate = int(sys.argv[2])
  else:
    print "Missing configuration file!"
    help_buffer = "Try \"%s cfg/test.cfg n\" (n = refresh rate in seconds)..." % sys.argv[0]
    print help_buffer
    exit(1)
  
  # Check if the file is readable
  if os.path.isfile(filename) and os.access(filename, os.R_OK):
    print "Configuration File exists and is readable ..."
  else:
    print "Either configuration file is missing or is not readable!"
    exit(1)
  
  # Check how many entries the file has
  device_count = 0
  with open(filename) as f:
    for i, l in enumerate(f):
      pass
  device_count = i + 1
  help_buffer = "%d device(s) found in configuration file ..." % device_count
  print help_buffer
  
if __name__ == "__main__":
  main(sys.argv[1:])

  # Refresh plot
  while (update_rate != 0):
    plot()
    plt.show(block=False)
    time.sleep(update_rate)
    plt.close()
  
  # Plot only once
  plot()
  plt.show(block=True)
