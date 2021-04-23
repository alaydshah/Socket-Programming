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

#include <bits/stdc++.h>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <queue>

#include "Server.h"

using namespace std;

#define HOST_NAME "localhost" // hostname
#define BACKLOG 10     // how many pending connections queue will hold
#define NUM_HOSPITALS 3

#define MAXDATASIZE 100 // max number of bytes we can get at once

void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);
void messageParser(string, string&, string&, string&);
int request_score(string);
void getInitialOccupancy(int);
string getClientRequest(const int, int*);
int sendLocationToHospital(int, string);
void receiveScores(int, int, string&);
void assign(int, int, string);

typedef pair<float, float> score;
enum Ports {UDP = 0, TCP, Hospital_A, Hospital_B, Hospital_C};
string hospital_names[3] = {"Hospital A", "Hospital B", "Hospital C"};
map<string, Ports> hospitalPort = { {"Hospital A",Hospital_A}, {"Hospital B",Hospital_B}, {"Hospital C", Hospital_C}};
const char* Port_Number[] = {"33819", "34819", "30819", "31819", "32819"};
map<string, int> book_keep;

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
        string assigned_hospital = "None";
        receiveScores(udp_sockfd, num_hospitals_sent, assigned_hospital);
        assign(child_sockfd, udp_sockfd, assigned_hospital);
        close(child_sockfd);
    }    
    return 0;
}

void getInitialOccupancy(int sockfd){

    for(int i=0; i<NUM_HOSPITALS; i++) {
        string message = server.receiveUDPPacket(sockfd);
        string sender, capacity, occupation;
        messageParser(message, sender, capacity, occupation);
        book_keep[sender] = stoi(capacity) - stoi(occupation);
        printf("The Scheduler has received information from %s: total capacity is %s and initial occupancy is %s\n", sender.c_str(), capacity.c_str(), occupation.c_str());
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
    printf("The Scheduler has received client at location %s from the client using TCP over port %s\n", location.c_str(), Port_Number[TCP]);
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
            server.sendUDPPacket(udp_sockfd, msg.str(), Port_Number[hospitalPort[hospital]]);            
            printf("The Scheduler has sent client location to %s using UDP over port %s\n", hospital.c_str(), Port_Number[UDP]);
            counter++;
        }
    }
    return counter;
}

void receiveScores(int sockfd, int num_hospitals_sent, string& assigned_hospital) {
    float best_score = INT_MIN;
    float best_distance = INT_MAX;
    bool flag = false;

    for(int i=0; i<num_hospitals_sent; i++) {
        string message = server.receiveUDPPacket(sockfd);
        string sender, score, distance;
        messageParser(message, sender, score, distance);
        printf("The Scheduler has received map information from %s, the score = %s and the distance = %s\n", sender.c_str(), score.c_str(), distance.c_str());
        if (distance == "None") {
            assigned_hospital = "Not Found";
            flag = true;
        }
        if (score != "None" and !flag) {
            if (stof(score) > best_score) {
                best_score = stof(score);
                best_distance = stof(distance);
                assigned_hospital = sender;
            }
            else if (stof(score) == best_score && stof(distance) < best_distance) {
                best_distance = stof(distance);
                assigned_hospital = hospital_names[i];
            }
        }
    }
}

void assign(int tcp_sockfd, int udp_sockfd, string assigned_hospital) {

    printf("The Scheduler has assigned %s to the client\n", assigned_hospital.c_str());
    server.respondTCPRequest(assigned_hospital, tcp_sockfd);
    printf("The Scheduler has sent the result to client using TCP over port %s\n", Port_Number[TCP]);

    if (assigned_hospital != "None" && assigned_hospital != "Not Found") {
        server.sendUDPPacket(udp_sockfd, "Assigned", Port_Number[hospitalPort[assigned_hospital]]);
        book_keep[assigned_hospital] -= 1;
        printf("The Scheduler has sent the result to %s using UDP over port %s\n", assigned_hospital.c_str(), Port_Number[UDP]);
    }
}