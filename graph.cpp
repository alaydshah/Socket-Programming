#include <bits/stdc++.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <vector>
#include <map>
#include <set>

#include "graph.h"

using namespace std;

typedef pair<float, string> Pair;
typedef pair<string, string> Edge;
string MAP_FILE_NAME = "map.txt";

// Constructor
Graph::Graph(string s)
{
    this->constructGraph();
    this->runDijkstra(s);
}

void Graph::constructGraph() {
    /*
    Reads the map.txt file and constructs the graph representation out of it.
    Graph is represented using a adjacency list map, where keys are nodes and values are list of adjacent nodes.
    */
    string line;
    ifstream file(MAP_FILE_NAME);

    if(!file) {
        cout << "Cannot open input file.\n";
        return;
    }    
    while (getline(file, line)) {
        std::vector<std::string> result; 
        std::istringstream iss(line); 
        for(std::string line; iss >> line;) {
            result.push_back(line); 
        }
        this->addEdge(result[0], result[1], stod(result[2]));
    }
}

void Graph::addEdge(string u, string v, float w) {
    /*
    Adds weighted edge to the adjacency list map. Since edges are undirected, it adds edge for both side of nodes.
    */

    Edge edge = make_pair(u, v);

    // If both nodes have been encountered, that means we already visited that edge
    if (edges.find(edge) == edges.end()) {
        adjMap[u].push_back(make_pair(w, v));
        adjMap[v].push_back(make_pair(w, u));
        edges.insert(make_pair(u,v));
        edges.insert(make_pair(v,u));
        distMap[u] = INT_MAX;
        distMap[v] = INT_MAX;
    }
}

void Graph::runDijkstra(string src) {
    /*
    Dijkstra's Algorithm. 
    Kindly note that this algorithm implementation is partly inspired from the following source: 
    https://www.geeksforgeeks.org/dijkstras-shortest-path-algorithm-using-priority_queue-stl/
    */

    priority_queue< Pair, vector <Pair> , greater<Pair> > pq;
    pq.push(make_pair(0, src));
    distMap[src] = 0;
	std::set<string> visited;

	/* Looping till priority queue becomes empty (or all
	distances are not finalized) */
	while (!pq.empty())
	{
		// The first vertex in pair is the minimum distance
		// vertex, extract it from priority queue.
		// vertex label is stored in second of pair (it
		// has to be done this way to keep the vertices
		// sorted distance (distance must be first item in pair)
		string vertex = pq.top().second;
		pq.pop();
		visited.insert(vertex);
        auto adjList = adjMap[vertex];

        // Get all adjacent vertices of a vertex
        for (auto pair : adjMap[vertex]) {
            float weight = pair.first;
            string n_vertex = pair.second;

            //  If not visited before and there is shorter path to v through u.
            if ((visited.find(n_vertex) == visited.end()) && distMap[n_vertex] > distMap[vertex] + weight) {

                // Updating distance of v
				distMap[n_vertex] = distMap[vertex] + weight;
				pq.push(make_pair(distMap[n_vertex], n_vertex));
            }
        }
	}
}

bool Graph::vertexExists(string vertex) {
    /*
    Check if given vertex exists in graph or not.
    */
   if (distMap.count(vertex) == 0) {
       return false;
   }
   return true;
}

float Graph::getShortestPath(string destination) {
    /*
    distMap contains shortest path to any destination from source which is hospital location.
    Query distmap and return the distance which will be shortest path distance.
    */
    return distMap[destination];
}

void Graph::printGraph() {
    /*
    Prints the graph.
    Note that this function was just implemented and used for debugging purpose. It is not called anywhere in the final codebase.
    */
    map<string, list<Pair>>::iterator it;

    for (it = adjMap.begin(); it != adjMap.end(); it++)
    {
        for (auto pair : it->second) {
            std::cout << it->first
              << ':'
              << pair.first   
              << ':'
              << pair.second
              << std::endl;
        }
    }

	
	printf("Vertex Distance from Source\n");
    map<string, float>::iterator it2;

    for (it2 = distMap.begin(); it2 != distMap.end(); it2++)
    {
        std::cout << it2->first 
            << ':'
            << it2->second
            << std::endl;
    }

}
