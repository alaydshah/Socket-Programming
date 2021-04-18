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
        void sendUDPPacket(int, string, const char*);
        string receiveTCPRequest(int, int*);
        void respondTCPRequest(string, int);
        void *get_in_addr(struct sockaddr *sa);
};