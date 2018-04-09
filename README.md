# Time-Sync
Implementation of Berkeley's Time Synchronization Algorithm for multiple processes each with its own logical clock

### Requirements:
Develop an n-node distributed system that implements Berkeley's time synchronization algorithm.

Because it applies to a distributed systems, we don't really need an actual human oriented notion of time, as long as we can provide a 'happened before' relationship between processes, then we are good.  

For this example, a logical clock would suffice for our purpose.

The distributed system uses a logical clock to timestamp messages sent/received between nodes.

To start the distributed system, each node should synchronize its logical clock to the same value, 
based on which the ordering of events can be determined among the machines.

To simplify the design and testing, the distributed system will be emulated using multiple processes on a single machine.
Each process represents a machine and has a unique port number for communication.

Suppose the logical clock on each machine represents the number of messages that have been sent and received by the machine.
The said logical clock is actually a counter used by the process to count events.

Randomly initialize the logical clock of individual processes and use Berkeley's time synchronization algorithm to synchronize these clocks to the average clock.  You can select any process as the time daemon to initiate the clock synchronization.  

After the synchronization, each process prints out its logical clock to check the result of synchronization.

### How to Compile:
Type the following in the src directory:
```c
make compile
```
Then,
```c
make clean
```
This will generate one binary file named: time_sync

### How to Run:
For the server, type: **time_sync \<port number\> \<processId\>**
For example: **./time_sync 7001 1**

To test the simulation, open 4 command line windows/prompts.
For the first 3 command line prompts, run the servers by typing the following commands in each prompt respectively:
- ./time_sync 7002 2
- ./time_sync 7003 3
- ./time_sync 7004 4

This will activate the servers/processes on ports 7002, 7003, and 7004 respectively.

For the fourth commandline window/prompt, run the time synchronization process by typing the following:
- ./time_sync 7001 1

The process will send a request over a socket connection to each of the 3 servers running on ports 7002 to 7004, requesting for the logical clock values, they will respond and then the synchronizing process will calculate the average clock value and send the individual offset clock value relative to the calculated average to each of the servers.

The servers will then respond with their updated logical clcok values which will __ALL be the SAME__, this time around.
You can view all their reponses from the __same command prompt window__; i.e. the command prompt of the synchronizing process.
You can also view the individual output from each server in its respective command prompt window as well.

### How It Works
The second parameter on the command line for the program, the *'process ID'* is for two purposes:
1. To easily identify each running process, while running.
2. To determine the role of the process, a process ID of 1 is a synchronizing process, any value other than 1 is simply a server whose clock will be synchronized.


Please note the following:
1. The port numbers for each of the servers are fixed, in other words, they must be 7002, 7003, and 7004.
2. For the synchronizing/client process, the process ID has to have a value of 1: this is what determines that it is the synchronizing process.  A process ID other than 1 is a server process.
3. A synchronizing process is the process that synchronizes the logical clock of all the other processes; the other processes are simply servers listening for a connection.
4. Once the synchronization/client process is done synchronizing the logical clocks of the listening servers, it closes on its own, but the other servers still remain running and can accept new connection requests.
5. After running the synchronization/client process the first time, you can run it again to check the returned logical clock values from all the servers, they will be the SAME.
6. Each process starts with a pseudo-randomly generated logical clock value; in other words, the logical clocks of the processes/servers are not hard-coded, they change it time they are run.
