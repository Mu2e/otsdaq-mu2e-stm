*********************************************************
README to use frontend (FE) code for Erdem's visit to MCR
*********************************************************

- For testing, Erdem has created a bitfile named:

  fmc120_emulated_readout_pmu_access_250000_packets_83gap_orderlyexit.bit

.., which I expect him to use at MCR. This bitfile programs the FPGA to send a fixed number of packets (exactly 250000), with the sending of each packet seprarated by a gap of 83 ns. This is the shortest time period by which Erdem can separate the packets being sent. The ADC data being sent is emulated on the FPGA and is just an incrementing counter.

- The following simpler frontend script has been written specifically for this purpose and for Erdem's visit to MCR:

  $STMDAQ_ROOT/frontends/testFE.cc

This script is a version of the frontend code that expects to receive a fixed number of packets from the firmware. In a multi-threaded framework, it sets up the firmware and triggers the sending of a fixed number of packets, reads and writes all the packets it receives, and exits the frontend, safely shutting down the firmware.

- To run this code, you'll need to source $STMDAQ_ROOT/setup.sh. There are 3 MCR-specific IP addresses (that Erdem will set when installing the firmware at MCR) that must be set as environment variables in the setup script for the frontend to run correctly. They are:

    # Set the IP address for the ipbus interfacw
    export STM_IPBUS_IP="192.168.42.210"
    # Set the IP address for the 10Gb ethernet interface
    export STM_10G_IP="192.168.34.10"
    # Set the UDP IP address
    export STM_READ_IP="127.0.0.1"

The IP addresses that are currently listed for MCR are the same as the IP addresses set on daq1 at UCL. I have asked Erdem to keep the IP addresses the same for MCR, but if he cannot for whatever reason, these will have to be changed in setup.sh.

- The code can be built in the root directory with the "make" command and will produce the following executable:

  ./build/bin/testFE.exe

- Running this executable will do the following:

1) Initialise the frontend, setting up the firmware, UDP connection, binary file etc. 

2) Begin a run with a run number that simply set at the start of int main() as:
   
     int run_number = 1;

3) Start either a thread to trigger the 10G packet sending.

4) Start an exit thread that will listen continuously for an exit command and then safely shut down the firmware and the FE if it receives it.

5) Start a read thread to listen for and receive packets.

6) Start a write thread to write packets after they're received. There is no processing of the data between reading and writing. Packets are simply written to file as they come in.

7) Should the code run as expected, it will receive all sent packets, write all received packets to a binary file in $STMDAQ_ROOT/data, exit the FE code automatically, and print the efficiency of the code to the screen. On UCL daq1, it has been operating at 100% read and write efficiency almost always. It should definitely operate at 100% on mu2edaq1.

NOTE: this code expects 250000 packets. Should Erdem change this number, you will need to change the varaible:

      static const uint maxPackets = 250000;	

... which is globally defined near the top of the script. If Erdem sends data continuously, once the code reaches the defined maximum packet number it will exit the code and give you the efficiency stats.

NOTE 2: If the FE gets stucks for whatever reason, or doesn't autoatimatially shutdown, the exit thread will safely shutdown the FE by inputting a Ctrl+C command mid-running.
