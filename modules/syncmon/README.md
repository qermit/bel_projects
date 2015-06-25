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

Example Usage
-------------

1. Configure all devices:
  <pre>./configure.sh cfg/timing_devices_complete.cfg</pre>

2. Let all devices output a pulse per second (this script musst run all the time):
  <pre>./eca-multi-pps.sh cfg/timing_devices_complete.cfg

3. Start the monitor:
  <pre>./syncmon dev/ttyUSB0</pre>
