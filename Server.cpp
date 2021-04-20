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

#include "Server.h"

using namespace std;

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
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
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    int yes=1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to use IPv4
    if (!sockType.compare("UDP")) {
        hints.ai_socktype = SOCK_DGRAM;
    }
    else {
        hints.ai_socktype = SOCK_STREAM;
    }

    hints.ai_flags = AI_PASSIVE; // use my IP

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
        // printf("listener: got packet from %s\n",
            // inet_ntop(their_addr.ss_family,
            //     this->get_in_addr((struct sockaddr *)&their_addr),
            //     s, sizeof s));
        // printf("listener: packet is %d bytes long\n", numbytes);
        buf[numbytes] = '\0';
        string message = string(buf);
        return message;  
}

void Server::sendUDPPacket(int sockfd, string message, const char * port_number) {

        // int sockfd;
        struct addrinfo hints, *pAddr;
        int rv;
        int numbytes;
        char s[INET6_ADDRSTRLEN];

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; // set to AF_INET to use IPv4
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE; // use my IP        

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

        // printf("talker: sent %d bytes to %s\n", numbytes, s);
        freeaddrinfo(pAddr);
}

string Server::receiveTCPRequest(const int sockfd, int * child_sockfd) {
    int new_fd, numbytes;  // listen on sock_fd, new connection on new_fd
    struct sockaddr_storage their_addr; // connector's address information
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
    // printf("server: got connection from %s\n", s);

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
    int numbytes;  
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (send(sockfd, response.c_str(), strlen(response.c_str()), 0) == -1) {
        perror("send");
        ::exit(1);
    }
    // printf("Scheduler assigned %s to client", response.c_str());    
}

void * Server::get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}