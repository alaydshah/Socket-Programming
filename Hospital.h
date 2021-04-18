#include <string>
#include "Graph.h"

using namespace std;
class Hospital {
    private:
        float capacity, occupancy;
        string hospital, destination;
        Graph graph;
    public:
        Hospital(string, string, float, float);
        string getDistance(string);
        string computeScore(string);
        float getAvailability();
        void printGraph();
        void parseSchedulerMessage(string);
        void resolveQuery(string);
};