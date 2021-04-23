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

#include "server.h"

using namespace std;

void sigchld_handler(int s)
{
    // Note: This part of code was taken from Beej Socket Tutorial
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

Server::Server() {
    HOST_NAME = "localhost";
    MAXDATASIZE = 100;
    BACKLOG = 10;
}

const int Server::createSocket(string sockType, const char * port_number) {
    /*
    Creates a TCP or UDP socket based on the input socket Type and returns a file descriptor of the created socket.
    Note: This part of code was taken from Beej Socket Tutorial
    */
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    int yes=1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; 
    if (!sockType.compare("UDP")) {
        hints.ai_socktype = SOCK_DGRAM;
    }
    else {
        hints.ai_socktype = SOCK_STREAM;
    }

    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(HOST_NAME.c_str(), port_number, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (sockType.compare("TCP")) {
            if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                    sizeof(int)) == -1) {
                perror("setsockopt");
                continue;
            }        
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        fprintf(stderr, "scheduler: failed to bind socket\n");
        ::exit(2);
    }
    return sockfd;
}

string Server::receiveUDPPacket(int sockfd) {
        /*
        Receives UDP packet at the input socket file descriptor, and returns the message.
        Note: This part of code was taken from Beej Socket Tutorial
        */
        int numbytes;
        struct sockaddr_storage their_addr;
        char buf[MAXDATASIZE];
        socklen_t addr_len;
        char s[INET6_ADDRSTRLEN];        
        addr_len = sizeof their_addr;
        if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            ::exit(1);
        }
        buf[numbytes] = '\0';
        string message = string(buf);
        return message;  
}

void Server::sendUDPPacket(int sockfd, string message, const char * port_number) {
        /*
        Sends input message through UDP using the input socket file descriptor and desination port number.
        Note: This part of code was taken from Beej Socket Tutorial        
        */

        struct addrinfo hints, *pAddr;
        int rv;
        int numbytes;
        char s[INET6_ADDRSTRLEN];

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; 
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE;        

        if ((rv = getaddrinfo(HOST_NAME.c_str(), port_number, &hints, &pAddr)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            return;
        }

        inet_ntop(pAddr->ai_family, this->get_in_addr((struct sockaddr *)pAddr->ai_addr),
                s, sizeof s);


        if ((numbytes = sendto(sockfd, message.c_str(), strlen(message.c_str()), 0,
                pAddr->ai_addr, pAddr->ai_addrlen)) == -1) {
            perror("talker: sendto");
            freeaddrinfo(pAddr);
            ::exit(1);
        }

        freeaddrinfo(pAddr);
}

string Server::receiveTCPRequest(const int sockfd, int * child_sockfd) {
    /*
    Accepts the TCP connection request at the input socket file descriptor, populates the child_sockfd at the given address location and returns the message.
    Note: This part of code was taken from Beej Socket Tutorial
    */
    int new_fd, numbytes;  
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    struct sigaction sa;
    char buf[MAXDATASIZE];
    char s[INET6_ADDRSTRLEN];
    int rv;

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        ::exit(1);
    }    

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        ::exit(1);
    }    

    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
        perror("accept");
        ::exit(1);
    }

    inet_ntop(their_addr.ss_family,
        this->get_in_addr((struct sockaddr *)&their_addr),
        s, sizeof s);

    if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        ::exit(1);
    }
    *child_sockfd = new_fd;
    buf[numbytes] = '\0';  
    string location = string(buf);
    return location;       
}

void Server::respondTCPRequest(string response, int sockfd){
    /*
    Sends the response message to the scheduler through TCP using input socket file descriptor.
    Note: This part of code was taken from Beej Socket Tutorial
    */
    int numbytes;  
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (send(sockfd, response.c_str(), strlen(response.c_str()), 0) == -1) {
        perror("send");
        ::exit(1);
    }
}

void * Server::get_in_addr(struct sockaddr *sa)
{
    // Note: This part of code was taken from Beej Socket Tutorial
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
