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
#include <string>
#include <map>

#include "Hospital.h"

using namespace std;

Hospital::Hospital(string hospital, string location, float capacity, float occupancy) : graph(location){
    this->hospital = hospital;
    this->destination = location;
    this->capacity = capacity;
    this->occupancy = occupancy;
}

string Hospital::getDistance(string source) {
    float d = this->graph.getShortestPath(source);
    if (d > 0) {
        return to_string(d);
    }
    return "None";
}

float Hospital::getAvailability() {
    return (capacity - occupancy) / capacity;
}

void Hospital::printGraph() {
    this->graph.printGraph();
}

void Hospital::parseSchedulerMessage(string s) {

    string delimiter = ":";
    int split_index = s.find(delimiter);
    string token_1 = s.substr(0, split_index); // token is "scott"
    string location = s.substr(split_index+1, s.length()); // token is "scott"

    if (token_1 == "Query") {
        printf ("Hospital %s has received input from client at location %s \n", this->hospital.c_str(), location.c_str());
        this->resolveQuery(location);
    }
    else {
        cout << token_1 << endl;
    }

}

void Hospital::resolveQuery(string location) {
    if (!this->graph.vertexExists(location)) {
        cout << "Query Resolved: Location not found" << endl;
    }
    else {
        cout << "Query Resolve: Locaiton found" << endl;
    }
}

string Hospital::computeScore(string source) {
    float d = this->graph.getShortestPath(source);
    float a = this->getAvailability();

    if (!(d>0) || (a<0 || a>1)){
        return "None";
    }

    // cout << "capacity:" << capacity << " " << "occupancy:" << occupancy << endl;

    cout << "a:" << a << " " << "d:" << d << endl;
    float score = 1 / (d*(1.1-a));
    return to_string(score);
}
