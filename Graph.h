#include <map>
#include <set>
#include <string>
#include <list>

using namespace std;

typedef pair<float, string> Pair;
typedef pair<string, string> Edge;

class Graph
{
	// In a weighted graph, we need to store vertex
	// and weight pair for every edge
    string source;

    private:
        map<string, list<Pair>> adjMap;
        set<Edge> edges;
        map<string, float> distMap;

    public:
        Graph(string); // Constructor

        // function to add an edge to graph
        bool vertexExists(string);
        float getShortestPath(string);
        void printGraph();


    private:
        void constructGraph();
        void addEdge(string u, string v, float w);
        void runDijkstra(string);
};