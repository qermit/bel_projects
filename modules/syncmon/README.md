Synchronization Monitor for White Rabbit
========================================

Purpose
-------

This application is used to monitor the synchronization quality of White Rabbit nodes.

Configuration File Layout
-------------------------

Example files can be found at the cfg/ directory.

<pre>[device name] [ip address] [io number/connection] [cable length in meters]</pre>

The first entry has to be the measurement/reference node.

Console Output
--------------

The output will look like this:

<pre>
./syncmon: Latest TS                 Count     Offset ts0  MaxFuture  MinFuture  MaxPast  MinPast  Average
./syncmon: ----------------------------------------------------------------------------------------------------
./syncmon: ts0: 0000075263000000140  00000200
./syncmon: ts1: 0000075264000000060  00000200  -080ns      -080ns     -079ns     +000ns   +000ns   -79.889999ns
./syncmon: ts2: 0000075264000000120  00000200  +060ns      +000ns     +000ns     +060ns   +059ns   +59.775333ns
</pre>

The according signals will look like this:

<pre>
                      ___                   
ts0 _________________|   |__________________
          ___                               
ts1 _____|   |______________________________
                                ___         
ts2 ___________________________|   |________
</pre>

Example Usage (without Data Master)
-----------------------------------

1. Configure all devices:
  <pre>./configure.sh cfg/timing_devices_complete.cfg</pre>

2. Let all devices output a pulse per second (this script musst run all the time):
  <pre>./eca-multi-pps.sh cfg/timing_devices_complete.cfg</pre>

3. Start the monitor:
  <pre>./syncmon dev/ttyUSB0</pre>

Log Files
---------

This tool will create two kinds of log files in the log/ directory:

1. syncmon_dev_io{n}.log: Snapshot of the latest measurement (see "Console Output").

2. syncmon_dev_plot_io{n}.log: Contains Event Numbers and Timestamps (since the tools has been started), for data plotting.
