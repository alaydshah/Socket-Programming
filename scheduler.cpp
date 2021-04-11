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

using namespace std;

#define UDP_PORT "33819" // the port hospitals will be connecting to
#define TCP_PORT "34819"  // // the port client will be connecting to
// #define HSP_A_PORT "30819" // Hospital A port
// #define HSP_B_PORT "31819" // Hospital B port
// #define HSP_C_PORT "32819" // Hospital C port
#define HOST_NAME "localhost" // hostname
#define BACKLOG 10     // how many pending connections queue will hold
#define NUM_HOSPITALS 1

#define MAXDATASIZE 100 // max number of bytes we can get at once

void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);
int request_score(string);
int createSocket(string);
void getInitialOccupancy(int);
string getClientRequest(int, int*);
void sendLocationToHospital(string);
void receiveScores(int);
string makeAssignment(int*);
void sendAssignmentToClient(int, string);
void sendAssignmentToHospital(int);


enum Socket_Type { TCP, UDP };

enum {Hospital_A = 0, Hospital_B, Hospital_C};
const char* port_number[] = {"30819", "31819", "32819"};

int main(void)
{
    const int udp_sockfd = createSocket("UDP");
    const int tcp_sockfd = createSocket("TCP");

    getInitialOccupancy(udp_sockfd);
    int child_sockfd;
    string location = getClientRequest(tcp_sockfd, &child_sockfd);
    sendLocationToHospital(location);
    receiveScores(udp_sockfd);
    int hospital_loc;
    string assignment = makeAssignment(&hospital_loc);
    sendAssignmentToClient(child_sockfd, assignment);
    sendAssignmentToHospital(hospital_loc);
    return 0;
}

int createSocket(string socktype) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    char * port;
    int yes=1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to use IPv4
    if (!socktype.compare("UDP")) {
        cout << "UDP" << endl;
        hints.ai_socktype = SOCK_DGRAM;
        port = UDP_PORT;
    }
    else {
        hints.ai_socktype = SOCK_STREAM;
        port = TCP_PORT;
    }

    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(HOST_NAME, port, &hints, &servinfo)) != 0) {
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

        if (socktype.compare("TCP")) {
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

void getInitialOccupancy(int sockfd){

    for(int i=0; i<NUM_HOSPITALS; i++) {
        int numbytes;
        struct sockaddr_storage their_addr;
        char buf[MAXDATASIZE];
        socklen_t addr_len;
        char s[INET6_ADDRSTRLEN];        
        addr_len = sizeof their_addr;
        if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            cout << "DEBUG" << endl;                
            perror("recvfrom");
            ::exit(1);
        }
        printf("listener: got packet from %s\n",
            inet_ntop(their_addr.ss_family,
                get_in_addr((struct sockaddr *)&their_addr),
                s, sizeof s));
        printf("listener: packet is %d bytes long\n", numbytes);
        buf[numbytes] = '\0';
        printf("listener: packet contains \"%s\"\n", buf);
    }
}


string getClientRequest(int sockfd, int* child_sockfd) {

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

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        ::exit(1);
    }    

    while(1) { // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process
            char buf[MAXDATASIZE];
            close(sockfd); // child doesn't need the listener
            if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
                perror("recv");
                ::exit(1);
            }
            cout << "Successfully Forked" << endl;
            *child_sockfd=new_fd;
            buf[numbytes] = '\0';
            string location = string(buf);
            return location;                       
        }
        close(new_fd);  // parent doesn't need this        
    }
}

void sendLocationToHospital(string location) {

    for(int i=0; i<NUM_HOSPITALS; i++) {
        int sockfd;
        struct addrinfo hints, *servinfo, *p;
        int rv;
        int numbytes;
        char s[INET6_ADDRSTRLEN];

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; // set to AF_INET to use IPv4
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE; // use my IP

        cout << port_number[i] <<endl;
        if ((rv = getaddrinfo(HOST_NAME, port_number[i], &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            cout << "DEBUG" << endl;
            continue;
        }

        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype,
                    p->ai_protocol)) == -1) {
                perror("talker: socket");
                continue;
            }

            break;
        }

        if (p == NULL) {
            fprintf(stderr, "talker: failed to create socket\n");
            continue;
        }

        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
                s, sizeof s);

        freeaddrinfo(servinfo);

        if ((numbytes = sendto(sockfd, location.c_str(), strlen(location.c_str()), 0,
                p->ai_addr, p->ai_addrlen)) == -1) {
            perror("talker: sendto");
            ::exit(1);
        }

        printf("talker: sent %d bytes to %s\n", numbytes, s);
        close(sockfd);
    }
}

void receiveScores(int sockfd) {
    for(int i=0; i<NUM_HOSPITALS; i++) {
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
        printf("listener: got packet from %s\n",
            inet_ntop(their_addr.ss_family,
                get_in_addr((struct sockaddr *)&their_addr),
                s, sizeof s));
        printf("listener: received packet from %s packet is %d bytes long\n", s, numbytes);
        buf[numbytes] = '\0';
        printf("listener: packet contains \"%s\"\n", buf);
    }
}

string makeAssignment(int* hosp_loc) {
    *hosp_loc = 0;
    return "Hospital A";
}

void sendAssignmentToClient(int sockfd, string assignment) {
    int numbytes;  
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (send(sockfd, assignment.c_str(), strlen(assignment.c_str()), 0) == -1) {
        perror("send");
        ::exit(1);
    }
    printf("Scheduler assigned %s to client", assignment.c_str());
    close(sockfd);
}

void sendAssignmentToHospital(int hospital_loc) {

        int sockfd;
        struct addrinfo hints, *servinfo, *p;
        int rv;
        int numbytes;
        char s[INET6_ADDRSTRLEN];

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
        hints.ai_socktype = SOCK_DGRAM;

        if ((rv = getaddrinfo(HOST_NAME, port_number[hospital_loc], &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            return;
        }

        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype,
                    p->ai_protocol)) == -1) {
                perror("talker: socket");
                continue;
            }

            break;
        }

        if (p == NULL) {
            fprintf(stderr, "talker: failed to create socket\n");
            return;
        }

        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
                s, sizeof s);

        freeaddrinfo(servinfo);
        string message="Client Assigned";
        if ((numbytes = sendto(sockfd, message.c_str(), strlen(message.c_str()), 0,
                p->ai_addr, p->ai_addrlen)) == -1) {
            perror("talker: sendto");
            ::exit(1);
        }

        printf("talker: sent %d bytes to %s\n", numbytes, port_number[hospital_loc]);
        close(sockfd);
}


void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}