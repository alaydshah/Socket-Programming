//g++ -std=c++11 -o trial.o trial.cpp

#include <bits/stdc++.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <vector>
#include <map>
#include <set>
#include "Graph.h"

using namespace std;

typedef pair<float, string> Pair;
typedef pair<string, string> Edge;
string MAP_FILE_NAME = "map.txt";
float INF = 2147483647;

// Constructor
Graph::Graph(string s)
{
    this->constructGraph();
    this->runDijkstra(s);
    cout << "source: " << s << endl;
    // this->printGraph();
    // this->getShortestPath(d);
}

void Graph::constructGraph() {
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
        // cout << result[0] << "::" << result[1] << ":" << stof(result[2]) << endl;
        this->addEdge(result[0], result[1], stod(result[2]));
    }
}

void Graph::addEdge(string u, string v, float w)
{
    Edge edge = make_pair(u, v);

    // If both nodes have been encountered, that means we already visited that edge
    if (edges.find(edge) == edges.end()) {
        adjMap[u].push_back(make_pair(w, v));
        adjMap[v].push_back(make_pair(w, u));
        edges.insert(make_pair(u,v));
        edges.insert(make_pair(v,u));
        distMap[u] = INF;
        distMap[v] = INF;
    }
}

void Graph::runDijkstra(string src) {
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
		// // 'i' is used to get all adjacent vertices of a vertex
		// list< pair<int, int> >::iterator i;
        for (auto pair : adjMap[vertex]) {
            float weight = pair.first;
            string n_vertex = pair.second;

            if ((visited.find(n_vertex) == visited.end()) && distMap[n_vertex] > distMap[vertex] + weight) {
                // Updating distance of v
				distMap[n_vertex] = distMap[vertex] + weight;
				pq.push(make_pair(distMap[n_vertex], n_vertex));
            }
        }
	}
}

bool Graph::vertexExists(string vertex) {
   if (distMap.find(vertex) == distMap.end()) {
       return false;
   }
   return true;
}

float Graph::getShortestPath(string destination) {
    // cout << distMap[destination] << endl;
    return distMap[destination];
}

void Graph::printGraph() {
    map<string, list<Pair>>::iterator it;

    for (it = adjMap.begin(); it != adjMap.end(); it++)
    {
        for (auto pair : it->second) {
            std::cout << it->first    // string (key)
              << ':'
              << pair.first   // string's value 
              << ':'
              << pair.second
              << std::endl;
        }
    }

	// Print shortest distances stored in dist[]
	printf("Vertex Distance from Source\n");
    map<string, float>::iterator it2;

    for (it2 = distMap.begin(); it2 != distMap.end(); it2++)
    {
        std::cout << it2->first    // string (key)
            << ':'
            << it2->second
            << std::endl;
    }

}