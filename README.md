# Time-Sync
Implementation of Berkeley's Time Synchronization Algorithm for multiple processes each with its own logical clock

Requirements:
Develop an n-node distributed system that implements Berkeley's time synchronization algorithm.

Because it applies to a distributed systems, we don't really need an actual human oriented notion of time, as long as we can provide a 'happened before' relationship between processes, then we are good.  
For this example, a logical clock would suffice for our purpose.

The distributed system uses a logically clock to timestamp messages sent/received between nodes.

To start the distributed system, each node should synchronize its logical clock to the same initial value, 
based on which the ordering of events can be determined among the machines.

To simplify the design and testing, the distributed system will be emulated using multiple processes on a single machine.
Each process represents a machine and has a unique port number for communication.

Suppose the logical clock on each machine represents the number of messages that have been sent and received by the machine.
The said logical clock is actually a counter used by the process to count events.

Randomly initialize the logical clock of individual processes and use Berkeley's time synchronization algorithm to synchronize these clocks to the average clock.  You can select any process as the time daemon to initiate the clock synchronization.  

After the synchronization, each process prints out its logical clock to check the result of synchronization.

# How to Compile:

# How to Run:
