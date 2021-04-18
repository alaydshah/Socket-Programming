#include <iostream>
#include <string>
#include "Hospital.h"

using namespace std;

Server server;

#define UDP_PORT "31819"
#define HOSPITAL "B"

int main(int argc, char *argv[])
{

    if (argc != 4) {
        fprintf(stderr,"usage: ./hospitalB <location B> <total capacity B> <initial occupancy B>\n");
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