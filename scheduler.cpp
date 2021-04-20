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

#include "Server.h"

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
int sendLocationToHospital(int, string);
void receiveScores(int, int);
string makeAssignment(int*);
void sendAssignmentToClient(int, string);
void sendAssignmentToHospital(int, int);
void messageParser(string, string&, string&, string&);


// enum Socket_Type { TCP, UDP };
typedef pair<int, int> Occupancy;
enum Ports {UDP = 0, TCP, Hospital_A, Hospital_B, Hospital_C};
Ports hospitals[3]  = {Hospital_A, Hospital_B, Hospital_C};
string hospital_names[3] = {"Hospital A", "Hospital B", "Hospital C"};
const char* Port_Number[] = {"33819", "34819", "30819", "31819", "32819"};
map<string, int> book_keep;

// map<std::string,Ports> hospital_map = { {"A",Hospital_A}, {"B",Hospital_B}, {"C", Hospital_C}};

Server server;

int main(void)
{
    const int udp_sockfd = server.createSocket("UDP", Port_Number[UDP]);
    const int tcp_sockfd = server.createSocket("TCP", Port_Number[TCP]);
    cout << "The Scheduler is up and running\n";
    getInitialOccupancy(udp_sockfd);
    while (true) {
        int child_sockfd;
        string location = getClientRequest(tcp_sockfd, &child_sockfd);
        int num_hospitals_sent = sendLocationToHospital(udp_sockfd, location);
        receiveScores(udp_sockfd, num_hospitals_sent);
        int hospital_loc;
        string assignment = makeAssignment(&hospital_loc);
        sendAssignmentToClient(child_sockfd, assignment);
        cout << "Assignment Sent to Client" << endl;
        sendAssignmentToHospital(udp_sockfd, hospital_loc);
        cout << "Assignment Sent to Hospitals" << endl;
        close(child_sockfd);
    }    
    cout << "Exited While Loop" << endl;
    return 0;
}

void getInitialOccupancy(int sockfd){

    for(int i=0; i<NUM_HOSPITALS; i++) {
        string message = server.receiveUDPPacket(sockfd);
        string sender, capacity, occupation;
        messageParser(message, sender, capacity, occupation);
        book_keep[sender] = stoi(capacity) - stoi(occupation);
        printf("The Scheduler has received information from %s: total capacity is %s and initial occupancy is %s\n", sender.c_str(), capacity.c_str(), occupation.c_str());
        // printf("listener: packet contains \"%s\"\n", location.c_str());
    }
}

void messageParser(string s, string& sender, string& var1, string& var2 ) {
    
    int split_index_1 = s.find(":"), split_index_2 = s.find(",");
    
    sender = s.substr(0, split_index_1);
    split_index_1++;

    var1 = s.substr(split_index_1, split_index_2-split_index_1);
    split_index_2++;

    var2 = s.substr(split_index_2, s.length() - split_index_2);
}

string getClientRequest(const int sockfd, int * child_sockfd) {
    string location = server.receiveTCPRequest(sockfd, child_sockfd);
    printf("The Scheduler has received client at location %s from the client using TCP over port %s", location.c_str(), Port_Number[TCP]);
    return location;    
}

int sendLocationToHospital(int udp_sockfd, string location) {
    int counter = 0;
    for(int i=0; i<NUM_HOSPITALS; i++) {
        string hospital = hospital_names[i];
        int remaining_capacity = book_keep[hospital];
        if (remaining_capacity > 0) {
            std::stringstream msg;
            msg << "Query:" << location;
            server.sendUDPPacket(udp_sockfd, msg.str(), Port_Number[hospitals[i]]);            
            printf("The Schediler has sent client location to %s using UDP over port %s", hospital.c_str(), Port_Number[UDP]);
            counter++;
        }
    }
    return counter;
}

void receiveScores(int sockfd, int num_hospitals_sent) {
    for(int i=0; i<num_hospitals_sent; i++) {
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

void sendAssignmentToHospital(int udp_sockfd, int hospital_loc) {
        string message="Assigned";
        server.sendUDPPacket(udp_sockfd, message, Port_Number[hospitals[hospital_loc]]);        
}