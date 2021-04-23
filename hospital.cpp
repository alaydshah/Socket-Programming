#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include <iostream>
#include <sstream>
#include <string>
#include <map>

#include "hospital.h"

using namespace std;

#define SCHEDULER_PORT "33819"

Hospital::Hospital(string hospital, string location, int capacity, int occupancy, const char * port_number) : graph(location){
    this->hospital = hospital;
    this->destination = location;
    this->capacity = capacity;
    this->occupancy = occupancy;
    this->port = port_number;
    this->bootUp();
}

void Hospital::bootUp() {
    /*
    Three tasks on bootup:
    1. Creates a socket, 
    2. Print the expected boot up message.  
    3. Sending initial occupancy to scheduler through UDP
    */

    this->sockfd = server.createSocket("UDP", this->port);
    printf ("Hospital %s is up and running using UDP on port %s.\n", this->hospital.c_str(), this->port);
    this->sendInitialOccupancy();
}

void Hospital::sendInitialOccupancy() {
    /*
    Sends Initial occupancy to scheduler through UDP socket.
    */
    printf ("Hospital %s has total capacity %d and initial occupancy %d.\n", this->hospital.c_str(), this->capacity, this->occupancy);
    string msg = createMessage(to_string(this->capacity), to_string(this->occupancy));
    server.sendUDPPacket(sockfd, msg, SCHEDULER_PORT);
}

string Hospital::listen() {
    /*
    Listens on the respective UDP port (of respective hospital) expecting a message from scheduler.
    */
    string message = server.receiveUDPPacket(sockfd);
    return message;
}

string Hospital::createMessage(string var1, string var2) {
    /*
    Prepares the message to be sent to the scheduler in the format the scheduler is expected to parse.
    */
    std::stringstream msg;
    msg << "Hospital " << this->hospital << ":" << var1 << "," << var2;
    return msg.str();
}

void Hospital::act(string s) {
    /*
    The functions acts on message received from the scheduler. In short, it has either of the two actions to take:
        1. If it is a query, then resolve the location query by computing score and distance
        2. If it is a message informing that client was assigned to this hospital, then update the occupancy and availability.
    */
    string delimiter = ":";
    int split_index = s.find(delimiter);
    string token_1 = s.substr(0, split_index); 
    string location = s.substr(split_index+1, s.length());

    if (token_1 == "Query") {
        printf ("Hospital %s has received input from client at location %s \n", this->hospital.c_str(), location.c_str());
        this->resolveQuery(location);
    }
    else {
        this->updateAssignment();
    }
}

void Hospital::resolveQuery(string location) {
    /*
    The functions conducts following two steps for resolve a location query:
        1. Computes score and distance while handling edge cases and invalid requests
        2. Sends the computed score and distance back to server through UDP.
    */
    if (this->graph.vertexExists(location)) {
        float availability = this->getAvailability();
        string distance = this->getDistance(location);
        string score = this->computeScore(location);
        string msg = createMessage(score, distance);
        printf("Hospital %s has capacity = %d, occupation = %d, availability = %f\n", this->hospital.c_str(), this->capacity, this->occupancy, availability);
        printf("Hospital %s has found the shortest path to client, distance = %s\n", this->hospital.c_str(), distance.c_str());
        printf("Hospital %s has the score = %s\n", this->hospital.c_str(), score.c_str());
        server.sendUDPPacket(sockfd, msg, SCHEDULER_PORT);
        printf("Hospital %s has sent score = %s and distance = %s to the Scheduler\n", this->hospital.c_str(), score.c_str(), distance.c_str());
    }
    else {
        string msg = createMessage("None", "None");
        printf("Hospital %s does not have the location %s in map\n", this->hospital.c_str(), location.c_str());
        server.sendUDPPacket(sockfd, msg, SCHEDULER_PORT);        
        printf("Hospital %s has sent \"location not found\" to the Scheduler\n", this->hospital.c_str());
    }
}

string Hospital::getDistance(string source) {
    /*
    The heavy loading for this function which is to compute shortest path is in turn done by a function of the graph class.
    Based on the computed distance, it returns either distance or None.
    */
    float d = this->graph.getShortestPath(source);
    if (d > 0) {
        return to_string(d);
    }
    return "None";
}

float Hospital::getAvailability() {
    /*
    Computes availability based on capacity and occupancy.
    */
    return (float) (capacity - occupancy) / (float) capacity;
}

void Hospital::printGraph() {
    /*
    Prints the graph.
    Note that this function was just implemented and used for debugging purpose. It is not called anywhere in the final codebase.
    */
    this->graph.printGraph();
}

string Hospital::computeScore(string source) {
    /*
    Computes the score based on shortest path and availability.
    Score could be None if shortest path or availability go out of specified limits.
    */
    float d = this->graph.getShortestPath(source);
    float a = this->getAvailability();

    if (!(d>0) || (a<0 || a>1)){
        return "None";
    }

    float score = 1 / (d*(1.1-a));
    return to_string(score);
}

void Hospital::updateAssignment() {
    /*
    Updates the occupancy, computes new availability based on updated occupany and logs them both on terminal. 
    */
    occupancy++;
    float a = getAvailability();
    string availability;
    if (a  < 0 || a > 1) {
        availability = "None";
    }
    else {
        availability = to_string(a);
    }
    printf("Hospital %s has been assigned to a client, occupation is updated to %s, availability is updated to %s\n", this->hospital.c_str(), to_string(occupancy).c_str(), availability.c_str());
}
