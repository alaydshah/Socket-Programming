#include <string>
using namespace std;
class Server {
    private:
        string HOST_NAME;
        int MAXDATASIZE, BACKLOG;
    public:
        Server();
        const int createSocket(string, const char *);
        string receiveUDPPacket(int);
        void sendUDPPacket(string message, const char* port_number);
        string receiveTCPRequest(int, int*);
        void respondTCPRequest(string, int);
        void *get_in_addr(struct sockaddr *sa);
};