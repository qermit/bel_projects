Synchronization Monitor for White Rabbit
========================================

Purpose
-------

This application is used to monitor the synchronization quality of White Rabbit nodes or switches.

Configuration File Layout
-------------------------

Example files can be found at the cfg/ directory. A configuration file has to look like this:

<pre>
  [device name] [ip/device address] [io number/connection] [cable length in meters]
</pre>

The first entry has to be the measurement/reference node!

Compiling
---------

To compile the syncmon and oe-config application just use the makefile:

<pre>
  make all
</pre>

Console Output
--------------

The output of syncmon will look like this:

<pre>
  ./syncmon: Latest TS                 Count     Offset ts0  MaxFuture  MinFuture  MaxPast  MinPast  Average
  ./syncmon: ----------------------------------------------------------------------------------------------------
  ./syncmon: ts0: 0000075264000000140  00000200
  ./syncmon: ts1: 0000075264000000060  00000200  -080ns      -080ns     -079ns     +000ns   +000ns   -79.889999ns
  ./syncmon: ts2: 0000075264000000200  00000200  +060ns      +000ns     +000ns     +060ns   +059ns   +59.775333ns
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

Example Usage
-------------

* Start test with a fake Data Master (reference node will become a fake Data Master):
  <pre>./start-fake-data-master.sh cfg/timing_devices_complete.cfg</pre>

* Start test with an external Data Master:
  <pre>./start-external-data-master.sh cfg/timing_devices_complete.cfg udp/dev/data.master</pre>

* Start test with a Data Master that is already running:
  <pre>./start-node-test.sh cfg/timing_devices_complete.cfg</pre>

* Start external Data Master only (without any other configuration):
  <pre>./start-data-master.sh udp/dev/data.master</pre>

* Start switch test:
  <pre>./start-switch-test.sh cfg/timing_devices_complete.cfg</pre>

* Start Syncmon:
  <pre>./syncmon cfg/timing_devices_complete.cfg</pre>

* Plot the results (optional, n = refresh rate in seconds, 0 => plot only once): 
  <pre>./plot-results.py cfg/timing_devices_complete.cfg n</pre>

Node Monitoring
---------------

1. Run start script (external or fake data master or node test)

2. Run the monitor application

3. Plot data if wanted

Switch Monitoring
-----------------

1. Run the switch test start script

2. Run the monitor application

3. Plot data if wanted

Log Files
---------

This tool will create two kinds of log files in the log/ directory:

1. {reference_device}_syncmon_dev_io{n}.log: Snapshot of the latest measurement (see "Console Output").

2. {reference_device}_syncmon_dev_plot_io{n}.log: Contains Event Numbers and Timestamps (since the tools has been started), for data plotting.
