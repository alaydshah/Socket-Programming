# Socket-Programming

## Author:
**Name:** Alay Dilipbhai Shah <br>
**USC Student ID:** 4038948819

## Compile and Execute:

Kindly run the following command in the terminal to compile the code and generate executable files.

```
make all
```

Since it is a distributed client-server application, we need **five terminals** and should run the following commands in each terminal **in given order**:

```
./scheduler --> Terminal 1

./hospitalA <location A\> <total capacity A\> <initial occupancy A\> --> Terminal 2

./hospitalB <location B\> <total capacity B\> <initial occupancy B\> --> Terminal 3

./hospitalC <location C\> <total capacity C\> <initial occupancy C\> --> Terminal 4

./client <Location of the client> --> Terminal 5

```

Note that the hospitals expect `map.txt` input file. Kindly provide your map file in addition to the valid input arguments for successfully executing the code.

&nbsp;
## Architecture Details:

<img src = "https://user-images.githubusercontent.com/25189188/115893228-cd8c0300-a40c-11eb-8ecf-d76fb1268a08.png">

&nbsp;

* Implemented a computational offloading distributed system based on client-server architecture using **UDP and TCP** sockets.
  
* The backend server of `hospitals` parses input `map.txt` and performs computation based on  Dijkstra's algorithm for given client location further passing the results to the edge server a.k.a `scheduler`.

* Edge server in turn does all the book-keeping to create customized resource allocation depending on the `client` queries.

&nbsp;
## Code Structure:

### **Client:** `client.cpp` <br>
This file consists of all the logic for client to successfully establish connection with scheduler for querying its location and to receive a response for the same. Note that client establishes a TCP connection with the scheduler, actively waits for response and terminates itself as soon as it gets a response for the queried location. 

&nbsp;

### **Server:** `server.cpp`, &nbsp; `server.h`<br>
Designed a server class in order to ensure minimum repetition of code across 4 servers (three hospital backends and 1 scheduler acting as edge server). <br>


Following are the public functions offerred by the server class:
1. **createSocket**
2. **receiveUDPPacket**
3. **sendUDPPacket**
4. **receiveTCPRequest**
5. **respondTCPRequest**

Above functions will be called by `scheduler` and `hospital` servers in order to communicate through UDP and TCP across the network. In addition to following DRY practice, this way of designing further helps to keep all communication related socket functionalities in one place so the respective server files (scheduler and hospitals) have to just take care of parsing messages and computing metrics. 

&nbsp;

### **Scheduler [Edge Server]:** `scheduler.cpp` <br>
Scheduler acts as an edge server taking request from clients further coordinating with the hospital servers to assign client at the best possible location. It relies on `server` class for communication but has the necessary functions for book-keeping, decoding messages and lastly deciding the assignment based on received scores.

&nbsp;
### **Graph:** `graph.cpp` &nbsp; `graph.h` <br>
Graph class would be responsible for all the computations related to graph, starting from reading and parsing the `map.txt`, creating a data structure to store graph followed by running **Dijkstra's algorithm** to find shortest path. Since all three hospitals rely on the same graph computations, it was necessary to have a separate class and which all hospital servers can rely on.

&nbsp;
### **Hospitals [Backend Servers]:**
### Class: `hospital.cpp` &nbsp; `hospital.h` <br>
Continuing to follow DRY coding principle, designed a hospital class that in turn uses `server` class for communication, `graph` class for graph computations and takes care of all the things in between which are decoding the messages, computing the score and logging necessary statements. This method of design makes the code base scalable in true sense since the individual hospital servers ( i.e hospitalA, hospitalB and hospitalC) just need the driver code to call two functions which are:

1. **listen**
2. **act**

These two functions in turn takes care of all the code logic, computations as well as communication. Hence, using this class, we can scale up this logic not just to three but any number of hospital servers.

### Driver: `hospitalA.cpp` &nbsp; `hospitalB.cpp` &nbsp; `hospitalC.cpp`

These files rely on above explained `hospital` class for its entire functionality. It just acts as a driver code to initialze the class with input location, capacity, occupancy along with its hardcoded UDP port. 

The driver code infinitely calls the **listen** (blocking call) and **act** functions offerred by the hospital class infinitely until termination.

&nbsp;
## Communication Messages:

In order for the clients and server to decode the message successfully after every exchange, they follow a particular format for communication as described below:

### **Client -- Scheduler**
1. Client --> Server: 
   * "_<Vertex Index\>_"
  
2. Server --> Client: 
    * "_Hospital <A/B/C>_"
    * "_Not Found_"
    * "_None_"

### **Scheduler -- Hospital**
1. Scheduler --> Hospital: 
    * "_Query:<Verted Index\>_"
    * "_Assigned_"
  
2. Hospital --> Scheduler: 
    * "_Hospital <A/B/C>:<occupancy\>,<capacity\>_"
    * "_Hospital <A/B/C>:<score\>,<distance\>_"

Note, all messages are of type **string**.

&nbsp;

## Re-used Code:
1. `client.cpp`: <br>
Most of the code below related to socket TCP communication were taken from **Beej Socket Tutorial.**

2. `server.cpp`: <br>
This class was created to separate communication functions from rest of the code and have it in all place. Most of the functionalities in this class are implemented using code snippets from **Beej Socket Tutorial.**

1. `graph.cpp`: <br>
This class was created to separate graph related computations from other computations. Shortest path distance computation which was done using **Dijkstra's algorithm** was majorly inspired from this source: https://www.geeksforgeeks.org/dijkstras-shortest-path-algorithm-using-priority_queue-stl/.

