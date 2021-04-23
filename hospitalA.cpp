#include <iostream>
#include <string>
#include "hospital.h"

using namespace std;

Server server;

#define UDP_PORT "30819"
#define HOSPITAL "A"

int main(int argc, char *argv[])
{

    if (argc != 4) {
        fprintf(stderr,"usage: ./hospitalA <location A> <total capacity A> <initial occupancy A>\n");
        exit(1);
    }    

    string location = argv[1];
    int capacity = stoi(argv[2]);
    int occupancy = stoi(argv[3]);

    Hospital hospital (HOSPITAL, location, capacity, occupancy, UDP_PORT);

    while (true) {
        string message = hospital.listen(); // Listen for scheduler message
        hospital.act(message);              // Act on the scheduler message
    }

    return 0;
}
