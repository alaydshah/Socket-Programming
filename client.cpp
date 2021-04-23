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

using namespace std;

#define SCH_PORT "34819" // the port client will be connecting to 
#define HOSTNAME "localhost"
#define MAXDATASIZE 100 // max number of bytes we can get at once 


void *get_in_addr(struct sockaddr *sa);

/*
The main job of the client is to:
1. Send Location to scheduler
2. Wait for response
3. When it gets one, it logs necessary messages in required format based on response it got.

Most of the code below related to socket TCP communication were taken from Beej Socket Tutorial.
*/

int main(int argc, char *argv[])
{
    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr,"usage: client <location of the client>\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(HOSTNAME, SCH_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    
    printf("The client is up and running\n");

    freeaddrinfo(servinfo); // all done with this structure

    if (send(sockfd, argv[1], strlen(argv[1]), 0) == -1) {
        perror("send");
        exit(1);
    }

    printf("The client has sent query to Scheduler using TCP: client location %s\n", argv[1]);

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    string assignment(buf);

    if (assignment == "Not Found") {
        printf("The client has received results from the Scheduler: assigned to Hospital None\n");
        printf("Location %s not found\n", argv[1]);
    }
    else if (assignment == "None") {
        printf("The client has received results from the Scheduler: assigned to Hospital None\n");
        printf("Score = None, No assignment\n");
    }
    else {
        printf("The client has received results from the Scheduler: assigned to %s\n", buf);
    }

    close(sockfd);

    return 0;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}