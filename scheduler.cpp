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

#include "Server.h"

#include <iostream>
#include <string>
#include <map>

using namespace std;

#define HOST_NAME "localhost" // hostname
#define BACKLOG 10     // how many pending connections queue will hold
#define NUM_HOSPITALS 3

#define MAXDATASIZE 100 // max number of bytes we can get at once

void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);
int request_score(string);
void getInitialOccupancy(int);
string getClientRequest(const int, int*);
void sendLocationToHospital(string);
void receiveScores(int);
string makeAssignment(int*);
void sendAssignmentToClient(int, string);
void sendAssignmentToHospital(int);


// enum Socket_Type { TCP, UDP };

enum Ports {UDP = 0, TCP, Hospital_A, Hospital_B, Hospital_C};
Ports hospitals[3]  = {Hospital_A, Hospital_B, Hospital_C};
const char* Port_Number[] = {"33819", "34819", "30819", "31819", "32819"};

Server server;

int main(void)
{

    const int udp_sockfd = server.createSocket("UDP", Port_Number[UDP]);
    const int tcp_sockfd = server.createSocket("TCP", Port_Number[TCP]);
    getInitialOccupancy(udp_sockfd);
    while (true) {
        int child_sockfd;
        string location = getClientRequest(tcp_sockfd, &child_sockfd);
        sendLocationToHospital(location);
        receiveScores(udp_sockfd);
        int hospital_loc;
        string assignment = makeAssignment(&hospital_loc);
        sendAssignmentToClient(child_sockfd, assignment);
        cout << "Assignment Sent to Client" << endl;
        sendAssignmentToHospital(hospital_loc);
        cout << "Assignment Sent to Hospitals" << endl;
        close(child_sockfd);
    }    
    cout << "Exited While Loop" << endl;
    return 0;
}

void getInitialOccupancy(int sockfd){

    for(int i=0; i<NUM_HOSPITALS; i++) {
        string location = server.receiveUDPPacket(sockfd);
        printf("listener: packet contains \"%s\"\n", location.c_str());
    }
}

string getClientRequest(const int sockfd, int * child_sockfd) {
    string location = server.receiveTCPRequest(sockfd, child_sockfd);
    cout << "Client is at location: " << location << endl;
    return location;    
}

void sendLocationToHospital(string location) {
    for(int i=0; i<NUM_HOSPITALS; i++) {
        server.sendUDPPacket(location, Port_Number[hospitals[i]]);
    }
}

void receiveScores(int sockfd) {
    for(int i=0; i<NUM_HOSPITALS; i++) {
        string score = server.receiveUDPPacket(sockfd);
        printf("Recieved Score: \"%s\"\n", score.c_str());
    }
}

string makeAssignment(int* hosp_loc) {
    *hosp_loc = 0;
    return "Hospital A";
}

void sendAssignmentToClient(int sockfd, string assignment) {
    server.respondTCPRequest(assignment, sockfd);
};

void sendAssignmentToHospital(int hospital_loc) {
        string message="Client Assigned";
        server.sendUDPPacket(message, Port_Number[hospitals[hospital_loc]]);        
}