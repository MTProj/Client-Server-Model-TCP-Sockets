#ifndef CLIENT_H_
#define CLIENT_H_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <map>
#include <vector>
#include <algorithm>

#include "../include/global.h"
#include "../include/client.h"
#include "../include/util.h"
#include "../include/networkutil.h"

using namespace std;

class client{

private:
    /*
        Client Variables
    */

    string listening_port_string;
    int port;

    int sock;                // Socket used for connection with server
    int inputsock;          // STDIN socket
    struct sockaddr_in addr; // Address struct

    fd_set master_list;     // set of file descriptors for reading
    bool loggedin;          // Logged in boolean
    bool hasLoggedFlag;     
    string AUTHOR;  

    string clientList;
    vector<string> clientIPVector;
    map<string,string> blockedClients;
    
    string left_over_msg;

public:

    client(); // Constructor
    void startClient(char portnumber[]); // Starts client on specified port # and initializes variables
    bool ipExistsLocally(string s);
    

    /* Server Message Parsers */
    void parseNEWLIST(const char*);
    void parseCLIENTIP(const char*);
    void parseNEWMESSAGE(string);
    void parseBACKLOGGEDMSG(const char* b);

    /* Set/Get Methods */
    void setListeningPort(string s);
    string getListeningPort();
    void setLoggedIn(bool b);
    bool getLoggedIn();
    void setClientList(string);
    string getClientList();
    void setClientIPVector(vector<string>);
    vector<string> getClientIPVector();
    string getIPAddress();
    map<string,string> getBlockedClients;

    /* Shell Commands */
    void doLOGIN(vector<string> tokenized,string cmd);
    void doLOGOUT(const char* input_buffer,string cmd);
    void doLIST(string cmd);
    void doREFRESH(const char* input_buffer,string cmd);
    void doSEND(const char* input_buffer,string cmd);
    void doBROADCAST(const char* input_buffer,string cmd);
    void doBLOCK(const char* input_buffer,string cmd);
    void doUNBLOCK(const char* input_buffer,string cmd);
    void doIP(string cmd);
    void doPORT(string cmd);
    void doAUTHOR(string cmd);
    void doEXIT(string cmd);
    void doERROR(string cmd);









};

#endif

