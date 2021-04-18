#include <string>
#include "Graph.h"
#include "Server.h"

using namespace std;

class Hospital {

    private:
        const char * port;
        int sockfd, capacity, occupancy;;
        string hospital, destination;
        Graph graph;
        Server server;

    public:
        Hospital(string, string, int, int, const char*);
        void bootUp();
        void sendInitialOccupancy();
        string listen();
        void act(string);
        void resolveQuery(string);
        string createMessage(string, string, int);
        string getDistance(string);
        string computeScore(string);
        float getAvailability();
        void printGraph();
        void updateAssignment();
};