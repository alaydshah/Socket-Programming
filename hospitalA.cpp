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

class Hospital {

    private:
        string HOST_NAME, SCHEDULER_PORT;
        int MAX_DATASIZE;
        string location, port;

    public:
        Hospital(string, string);
        void sendInitialOccupancy();
        int createSocket();
        string recieveLocationRequest(int);
        void sendScores(string);
        string receiveAssignment(int);

        void *get_in_addr(struct sockaddr *sa);
};

Hospital::Hospital(string loc, string port_num) {

    HOST_NAME = "localhost";    
    SCHEDULER_PORT = "33819";
    MAX_DATASIZE = 100;    
    location = loc;
    port = port_num;
}

void Hospital::sendInitialOccupancy() {
    
        int sockfd;
        struct addrinfo hints, *servinfo, *p;
        int rv;
        int numbytes;
        char s[INET6_ADDRSTRLEN];

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; // set to AF_INET to use IPv4
        hints.ai_socktype = SOCK_DGRAM;
        cout << location << endl;
        cout << SCHEDULER_PORT << endl;

        if ((rv = getaddrinfo(HOST_NAME.c_str(),SCHEDULER_PORT.c_str(), &hints, &servinfo)) != 0) {
            cout << "DEBUG" << HOST_NAME << endl;            
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

        if ((numbytes = sendto(sockfd, location.c_str(), strlen(location.c_str()), 0,
                p->ai_addr, p->ai_addrlen)) == -1) {
            perror("talker: sendto");

            ::exit(1);
        }
        cout << "sent" << endl;
        printf("talker: sent %d bytes to %s\n", numbytes, s);
        close(sockfd);
}

int Hospital::createSocket() {

    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(HOST_NAME.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
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
    cout << "Hospital will wait at " << HOST_NAME << port << endl;        
    return sockfd;
}

void* Hospital::get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

string Hospital::recieveLocationRequest(int sockfd) {
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAX_DATASIZE];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];        

    addr_len = sizeof their_addr;
    cout << "Hospital Waiting For Location Request" << endl;
    if ((numbytes = recvfrom(sockfd, buf, MAX_DATASIZE-1 , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        ::exit(1);
    }
    printf("Hospital: got packet from %s\n",
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s));
    printf("Hospital: received packet from %s packet is %d bytes long\n", s, numbytes);
    buf[numbytes] = '\0';
    printf("Hospital: packet contains \"%s\"\n", buf);
}

void Hospital::sendScores(string score) {
    
        int sockfd;
        struct addrinfo hints, *servinfo, *p;
        int rv;
        int numbytes;
        char s[INET6_ADDRSTRLEN];

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; // set to AF_INET to use IPv4
        hints.ai_socktype = SOCK_DGRAM;

        if ((rv = getaddrinfo(HOST_NAME.c_str(), SCHEDULER_PORT.c_str(), &hints, &servinfo)) != 0) {
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

        if ((numbytes = sendto(sockfd, score.c_str(), strlen(score.c_str()), 0,
                p->ai_addr, p->ai_addrlen)) == -1) {
            perror("talker: sendto");
            ::exit(1);
        }

        printf("talker: sent %d bytes to %s\n", numbytes, s);
        close(sockfd);
}


string Hospital::receiveAssignment(int sockfd) {
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAX_DATASIZE];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];        

    addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, buf, MAX_DATASIZE-1 , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        ::exit(1);
    }
    printf("Hospital: got assignment from %s\n",
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s));
    printf("Hospital: received packet from %s packet is %d bytes long\n", s, numbytes);
    buf[numbytes] = '\0';
    printf("Hospital: packet contains \"%s\"\n", buf);
}

#define HOST_NAME "localhost" // hostname
#define SCHEDULER_PORT "33819" // hostname

int main(void)
{
    Hospital hospital_A ("A", "30819");
    hospital_A.sendInitialOccupancy();
    cout << "Sent" <<endl;
    int sockfd = hospital_A.createSocket();
    hospital_A.recieveLocationRequest(sockfd);
    hospital_A.sendScores("Score: 0");
    hospital_A.receiveAssignment(sockfd);
    return 0;
}

