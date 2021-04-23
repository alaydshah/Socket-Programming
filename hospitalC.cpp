#include <iostream>
#include <string>
#include "hospital.h"

using namespace std;

Server server;

#define UDP_PORT "32819"
#define HOSPITAL "C"

int main(int argc, char *argv[])
{

    if (argc != 4) {
        fprintf(stderr,"usage: ./hospitalC <location C> <total capacity C> <initial occupancy C>\n");
        exit(1);
    }    

    string location = argv[1];
    int capacity = stoi(argv[2]);
    int occupancy = stoi(argv[3]);

    Hospital hospital (HOSPITAL, location, capacity, occupancy, UDP_PORT);

    while (true) {
        string message = hospital.listen();
        hospital.act(message);
    }

    return 0;
}