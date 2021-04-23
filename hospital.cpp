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
    this->sockfd = server.createSocket("UDP", this->port);
    printf ("Hospital %s is up and running using UDP on port %s.\n", this->hospital.c_str(), this->port);
    this->sendInitialOccupancy();
}

void Hospital::sendInitialOccupancy() {
    printf ("Hospital %s has total capacity %d and initial occupancy %d.\n", this->hospital.c_str(), this->capacity, this->occupancy);
    string msg = createMessage(to_string(this->capacity), to_string(this->occupancy));
    server.sendUDPPacket(sockfd, msg, SCHEDULER_PORT);
}

string Hospital::listen() {
    string message = server.receiveUDPPacket(sockfd);
    return message;
}

string Hospital::createMessage(string var1, string var2) {
    std::stringstream msg;
    msg << "Hospital " << this->hospital << ":" << var1 << "," << var2;
    return msg.str();
}

void Hospital::act(string s) {
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
    float d = this->graph.getShortestPath(source);
    if (d > 0) {
        return to_string(d);
    }
    return "None";
}

float Hospital::getAvailability() {
    return (float) (capacity - occupancy) / (float) capacity;
}

void Hospital::printGraph() {
    this->graph.printGraph();
}

string Hospital::computeScore(string source) {
    float d = this->graph.getShortestPath(source);
    float a = this->getAvailability();

    if (!(d>0) || (a<0 || a>1)){
        return "None";
    }

    float score = 1 / (d*(1.1-a));
    return to_string(score);
}

void Hospital::updateAssignment() {
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
