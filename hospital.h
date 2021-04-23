#include <string>
#include "graph.h"
#include "server.h"

using namespace std;

class Hospital {

    private:
        const char * port;
        int sockfd, capacity, occupancy;;
        string hospital, destination;
        Graph graph;
        Server server;
        void bootUp();
        void sendInitialOccupancy();
        void resolveQuery(string);
        string createMessage(string, string);
        string getDistance(string);
        string computeScore(string);
        float getAvailability();
        void printGraph();
        void updateAssignment();

    public:
        Hospital(string, string, int, int, const char*);
        string listen();
        void act(string);
};
