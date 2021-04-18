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

#include "Server.h"

using namespace std;

Server server;

#define HOST_NAME "localhost" // hostname
#define SCHEDULER_PORT "33819" // hostname
#define UDP_PORT "32819"

int main(void)
{
    const int udp_sockfd = server.createSocket("UDP", UDP_PORT);
    server.sendUDPPacket(udp_sockfd, "C", SCHEDULER_PORT);
    cout << "Hospital C Sent Initial Occupancy to scheduler" <<endl;
    while (true) {
        string location_request = server.receiveUDPPacket(udp_sockfd);
        cout << "Hospital C Locating Request received from scheduler:" << location_request << endl;
        server.sendUDPPacket(udp_sockfd, "Score: 2", SCHEDULER_PORT);
        string assignment = server.receiveUDPPacket(udp_sockfd);
        cout << "Hospital C Assignment received from scheduler:" << assignment << endl;
    }
    return 0;
}