#ifndef SERVER_H_
#define SERVER_H_


#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <map>
#include <vector>
#include <sys/types.h>
#include <netdb.h>

#include "../include/util.h"
#include "../include/global.h"
#include "../include/networkutil.h"

using namespace std;

class server{
    private:
        /*
            Private class to hold client specific information
            ip address
            port
            storedMessages
         */
        class clientobj{
        public:
            int sock;
            int id;
            int port;
            string hostname;
            string ip;
            bool online;
            vector<string> storedMessages;
            map<string,string> blockedClients;
            int msgsrecvd;
            int msgssent;
            string left_over_msg;
        
            clientobj();

            void setSocket(int socket);
            int getSocket();
            void setIP(string ip);
            string getIP();
            void setPort(int port);
            int getPort();
            void setOnlineStatus(bool online);
            bool getOnlineStatus();
            void addMessages(string s);
            vector<string> getMessages();
            void setHostName(string s);
            string getHostName();
            void incMsgsRecvd();
            int getMsgsRecvd();
            void incMsgsSent();
            int getMsgsSent();
            void setIdentifier(int);
            int getIdentifier();
            void addBlockedClient(string ip);
            void removeBlockedClient(string ip);
            map<string,string> getBlockedClients();
            bool isClientBlocked(string ip);
            void setLeftOverMessage(string s);
            string getLeftOverMessage();
        };
        /*
            Server setup variables
        */
        int port; // port number passed in via command line when starting server
        int maxclients; // Maximum supported clients
        int server_socket; // Initial server socket - this socket will listen for new connections
        int inputsock; // stdin "socket" - this socket will be for stdin
        int onlineClients; // Count of number of online clients
        struct sockaddr_in server_addr; // Server address struct
        int saddr_len;

        const char* AUTHOR;


        fd_set master_list; // Holds all file descriptors for server - Listening,STDIN,Clients sockets.

        // Map to store console commands. This is used to easily identify which console command user entered in switch statement.        
        map<const char*,int> console_commands;

        /*
            Client Handling Variables
        */
        struct sockaddr_in client_addr; // Client address struct
        int caddr_len;  // Length of client address struct
        int client_socket;  // Socket for new client connecting
        char buffer[]; // Input buffer for client messages
        const char *msg;

        vector <pair<int,string> > clients_sockets; // This will hold all currently logged in client sockets.
        map<string,clientobj> clientMap; // This will hold all currently active clients. Those who have not "EXITED"
        vector<clientobj> totalClients; // This will hold all clients who have ever logged into server


        
        /*
            Error Handling Variables
        */
        int inval; // Int to store read() return



        
        

    public:
        
        server(); // Constructor
        void startServer(char portnumber[]); // Starts server on specified port # and initializes variables
        
        // Helper Functions
        void addClient(int clientsocket,string ip,int port,string host); // Adds new client to clientMap, totalClients and clients_sockets
        void logClientOut(int clientsocket); // 
        vector<string> tokenize(string s);
        string getListOfClients(); // Returns list of clients currently logged in - sorted by port number
        string getStatistics(); // Returns stats on all clients who have ever logged in
        string getListOfBlockedClients(string); // Returns list format of blocked clients
        void addToClientMap(clientobj client); // Called when a new client has connected
        void removeFromClientMap(clientobj client); // Called when a client has sent EXIT
        vector <pair<int,string> > selectionSortArray(vector<pair<int,string> >);
        string getOnlineIpAddresses();
        string getIPAddress();
        bool ipExists(string ip);
        void sendListAndIPs(int socket);

        // Server shell commands
        void doAUTHOR(string cmd);
        void doIP(string cmd);
        void doPORT(string cmd);
        void doLIST(string cmd);
        void doSTATISTICS(string cmd);
        void doBLOCKED(const char* input_buffer,string cmd);
        void doERROR(string cmd);
        
        // Parsing incoming message_types
        void parseREFRESH(pair<int,string> activeClient);
        void parseLOGOUT(pair<int,string> activeClient);
        void parseEXIT(pair<int,string> activeClient);
        void parseSEND(pair<int,string> activeClient,string input);
        void parseBROADCAST(pair<int,string> activeClient,const char* input_buffer);
        void parseBLOCK(pair<int,string> activeClient,const char* input_buffer);
        void parseUNBLOCK(pair<int,string> activeClient,const char* input_buffer);
        void parseNEWCLIENT(pair<int,string> activeClient,const char* input_buffer);

        
};


#endif