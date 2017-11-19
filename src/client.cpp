#include "../include/client.h"
#include "../include/logger.h"

client::client()
{}

void client::startClient(char p[]){
    /*
        Initialize Variables
    */
    left_over_msg = "";

    listening_port_string = p;
    loggedin = false;
    port = atoi(p);

    memset(&addr,'0',sizeof(addr));

    addr.sin_family = AF_INET;
    
    int ready;
    int maxfd;
    int inval;

    string cmd;
    vector<string> tokenized;

    util util;

    clientList = "";
    vector<string> clientIPVector;
    inputsock = fileno(stdin);

    
    
    while(true){
        /*
            Initialize FD_SETs and find max fd for both sets
        */
        FD_ZERO(&master_list);          // Clear Set
        FD_SET(inputsock,&master_list); // Add input "socket" to set. This is just std in to accept user input
        FD_SET(sock,&master_list);      // Add socket to set
        maxfd = sock;
        if(sock < maxfd){ 
            maxfd = inputsock; 
        }
        /*
            Call select() - Blocks until activity. If ready < 0 , then there was an error.
            When there is activity FD_ISSET is used to check which socket (file descriptor) is ready.
            It then reads the input
        */
        ready = select(maxfd+1,&master_list,NULL,NULL,NULL);
        if(ready < 0){ 
            
        }

        /*
            Check if STDIN is active
        */
        if(FD_ISSET(inputsock,&master_list) == true){

            /* Allocate Buffer */
            int bufsize = 1025;
            char *input_buffer = new char[bufsize];
            size_t sbuf = sizeof(input_buffer);

            inval = read(inputsock,input_buffer,bufsize);
            input_buffer[strlen(input_buffer)] = '\0';

            int i = 0;
            string m = "";
            while(input_buffer[i] != '\n'){
                m = m + input_buffer[i];
                i++;
            }
            string temp = m;

            /* Delete buffer */
            delete [ ] input_buffer;
        
            if(inval < 0 || inval == 0){
                
            }else{

                //Check if command is single command.
                string s = temp;
                int t = s.find(" ");

                if(t == -1){
                    cmd = util.stringClean(temp.c_str());
                }else{
                    tokenized = util.stringTokenize(temp.c_str());
                    cmd = tokenized[0];
                    cmd = util.stringClean(cmd.c_str());
                }

                //Client logged in to server
                if(loggedin){
                    if(cmd == "LIST"){
                        doLIST(cmd);
                    }else if(cmd == "REFRESH"){
                        doREFRESH(temp.c_str(),cmd);                        
                    }else if(cmd == "SEND"){
                        doSEND(temp.c_str(),cmd);
                    }else if(cmd == "BROADCAST"){
                        doBROADCAST(temp.c_str(),cmd);
                    }else if(cmd == "BLOCK"){
                        doBLOCK(temp.c_str(),cmd);
                    }else if(cmd == "UNBLOCK"){
                        doUNBLOCK(temp.c_str(),cmd);
                    }else if(cmd == "LOGOUT"){  
                        doLOGOUT(temp.c_str(),cmd);    
                        FD_CLR(sock,&master_list);          
                    }else if(cmd == "IP"){
                        doIP(cmd);
                    }else if(cmd == "PORT"){
                        doPORT(cmd);
                    }else if(cmd == "EXIT"){
                        doEXIT(cmd);
                    }else if(cmd == "AUTHOR"){
                        doAUTHOR(cmd);
                    }else{
                        doERROR(cmd);
                    }

                //Client is NOT Logged into server
                }else{
                    if(cmd == "AUTHOR"){
                        doAUTHOR(cmd);
                    }else if(cmd == "IP"){
                        doIP(cmd);
                    }else if(cmd == "PORT"){
                        doPORT(cmd);
                    }else if(cmd == "LOGIN"){
                        doLOGIN(tokenized,cmd);
                    }else if(cmd == "EXIT"){  
                        doEXIT(cmd);                 
                    }else{
                        doERROR(cmd);
                    }
                }
            }
 
            
        /*
            Check if the listening socket is active
            this is the socket connected with the server
        */
        }else if(FD_ISSET(sock,&master_list)== true){
            int bufsize = 1025;
            char *input_buffer = new char[bufsize];

            vector<string> tokenizedMessage;

            /* READ DATA FROM SOCKET */
            networkutil network;
            inval = read(sock,input_buffer,bufsize);
            
            if(inval == 0){
                //No connection on socket
            }else{

                /* Check to make sure entire message is there */
                string s = input_buffer;
                bool disconnect = false;
                /* Continue reading until beginmsg and endmsg are found */
                while(s.find("<BEGIN_MSG>") == -1 || s.find("<END_MSG>") == -1){
                    inval = read(sock,input_buffer,bufsize);
                    if(inval == 0){
                        //cout << "Lost connection to server\n";
                        bool disconnect = true;
                        break;
                    }
                    string newmsg = input_buffer;                     
                    s = s + newmsg;
                    
                }

                /* Delete buffer */
                delete [ ] input_buffer;

                /* Make sure there was no disconnection while reading again*/
                if(disconnect == false){
                    /* Parse Message */
                    string temp = network.parseFromSocket(s);
                    tokenizedMessage = util.stringTokenize(temp.c_str());
                    string message_type = tokenizedMessage[0];

                    if(message_type  == "NEWLIST"){
                        parseNEWLIST(temp.c_str());
                    }else if(message_type == "NEWMESSAGE"){
                        parseNEWMESSAGE(temp.c_str());
                    }else if(message_type == "BACKLOGGED_MESSAGES"){
                        parseBACKLOGGEDMSG(temp.c_str());
                    }
                }
                
            }
        }          
    }
}
/*  Message Parse Functions #*/
/*

    Parse message from server NEWLIST. Server will send an updated client list 
    when client connects, or calls refresh.

    Message Format:
    <NEWLIST> <list string> 
    <CLIENTIP> <ip> <ip> <ip>
*/
void client::parseNEWLIST(const char* b){
    util u;

    // Parse NEWLIST
    string buf = b;
    string message_type = "CLIENTIP ";
    int size = strlen(message_type.c_str());

    int start = buf.find(" ") + 1;
    int end = buf.find("CLIENTIP ") - size;
    string list = buf.substr(start,end) + "\n"; // Remove <NEWLIST>
    setClientList(list);


    //Parse CLIENTIP
    start = buf.find("CLIENTIP ");
    string ips = buf.substr(start + 1);

    vector<string> iplist;
    vector<string> tokenized = u.stringTokenize(ips.c_str());

    // Start at 1 because the first string in tokenized is the message_type
    for(int i = 1; i < tokenized.size(); i++){
        //coutcout << "Adding '" << tokenized[i] << "' to clientIPVector.\n";
        iplist.push_back(tokenized[i]);
    }
    setClientIPVector(iplist);
}

/*
    Parse Message From Server BACKLOGGED_MESSAGES. The server will send this message
    when a client re-logs into the server.

    Format of Message:
    <BACKLOGGED_MESSAGES> <Message1> <Message2> <Message3>
*/
void client::parseBACKLOGGEDMSG(const char* b){

    vector<string> tokenized;

    string buf = b;
    util u;

    // Remove the message type
    string msg = "";
    msg = buf.substr(buf.find(" ") + 1);


    // tokenize messages
    string m = "";
    for(int d = 0; d < strlen(msg.c_str()); d++){
        char s = msg[d];
        if(s == '\n'){
            m = m + s;
            tokenized.push_back(m);
            m = "";
        }else{
            m = m + s;
        }


    }

    // Tokenize the message by newline char
    // Print each message
    // Format: <FromIP> <msg>
    
    for(int i = 0; i < tokenized.size(); i++){
        string s = tokenized[i];
        string msg = "";
        string from = "";

        s = s.substr(s.find(" ") + 1);

        from = s.substr(0,s.find(" "));
        msg = s.substr(s.find(" ") + 1);

        string message_received_output = "";
        message_received_output = "msg from:" + from + "\n" + "[msg]:" + msg;
        string command = "RECEIVED";
        printf("[%s:SUCCESS]\n",command.c_str());
        printf(message_received_output.c_str());
        printf("[%s:END]\n",command.c_str());
    }
    
}
/*
    Parse incoming server message NEWMESSAGE. Server will forward messages to this
    client from other clients.

    Message Format:<NEWMESSAGE> <from ip address> <message>

*/
void client::parseNEWMESSAGE(string b){
    string buf = b;
    util u;
    string msg = "";
    string from = "";

    msg = buf.substr(buf.find(" ") + 1);         //Remove <NEWMESSAGE>
    from = msg.substr(0,msg.find(" "));        //Save fromIP
    msg = msg.substr(msg.find(" " ) + 1);        //Remove <from ip>

    string message_received_output = "";
    message_received_output = "msg from:" + from + "\n" + "[msg]:" + msg;
    string command = "RECEIVED";
    printf("[%s:SUCCESS]\n",command.c_str());
    printf(message_received_output.c_str());
    printf("\n[%s:END]\n",command.c_str());
}










/* Shell Command Functions */
/*
    Sends LOGIN message to server. 
    Sends NEWCLIENT message to server as well. This NEWCLIENT message notifys the server
    to update the listening port of this client. 

    Server responds with NEWLIST message
    Server may respond with BACKLOGGED_MESSAGES if client was previously logged in.
    
*/

void client::doLOGIN(vector<string> tokenized,string cmd){
    util util;
    
    if(tokenized.size() != 3){
        printf("[%s:ERROR]\n",cmd.c_str());
        printf("[%s:END]\n",cmd.c_str());
    }else if(util.isValidIP(tokenized[1]) == false || util.isValidPort(tokenized[2]) == false){
        printf("[%s:ERROR]\n",cmd.c_str());
        printf("[%s:END]\n",cmd.c_str());
    }else{

        int p = atoi(tokenized[2].c_str());
        addr.sin_port = htons(p);
        addr.sin_addr.s_addr = inet_addr(tokenized[1].c_str());

        sock = socket(AF_INET,SOCK_STREAM,0);
        if(sock < 0){

        }
        int in = connect(sock,(struct sockaddr*)&addr,sizeof(addr));
        if(in == -1){
            printf("[%s:ERROR]\n",cmd.c_str());
            close(sock);
            sock = 0;
            printf("[%s:END]\n",cmd.c_str());
        }else{
            printf("[%s:SUCCESS]\n",cmd.c_str());

            string message_type = "NEWCLIENT ";
            string s2 = getListeningPort();
            string m = message_type + s2;
            size_t s_m = strlen(m.c_str());
            
            networkutil network;
            network.sendOverSocket(sock,m.c_str());
            setLoggedIn(true);
            printf("[%s:END]\n",cmd.c_str());
        }
    }
}
/*
    Sends LOGOUT message to server. Server and client close socket. 
    All information for client is saved on the server though and the 
    client can log back in. 
*/
void client::doLOGOUT(const char* input_buffer,string cmd){
    printf("[%s:SUCCESS]\n",cmd.c_str());

    size_t bufsize = strlen(input_buffer);

    networkutil network;
    network.sendOverSocket(sock,input_buffer);
    close(sock);
    sock = 0;
    setLoggedIn(false);
    printf("[%s:END]\n",cmd.c_str());
}
/*
    DOES NOT SEND MESSAGE TO SERVER

    Simply executes LIST , displaying contents of clientList
    which are the locally known online clients.
*/
void client::doLIST(string cmd){
    printf("[%s:SUCCESS]\n",cmd.c_str());
    printf(getClientList().c_str());
    printf("[%s:END]\n",cmd.c_str());
}
/*
    Sends REFRESH message to server. This will update the local copy
    of the clientList and clientVectorIP. The server will respond with
    NEWLIST message.

    Format
    <REFRESH>
*/
void client::doREFRESH(const char* input_buffer,string cmd){
    printf("[%s:SUCCESS]\n",cmd.c_str());
    size_t bufsize = strlen(input_buffer);

    networkutil network;
    network.sendOverSocket(sock,input_buffer);

    printf("[%s:END]\n",cmd.c_str());
}
/*
    Send message_type: SEND to server. This is used for client to client communication.
    All messages are passed through the server then onto the client depending.

    Format: 
    <SEND> <DESTINATION IP> <MSG>
*/
void client::doSEND(const char* input_buffer,string cmd){
    util util; 
    vector<string> tokenized = util.stringTokenize(input_buffer);


    if(tokenized.size() >=2 == true){
        if(util.isValidIP(tokenized[1]) == true && ipExistsLocally(tokenized[1]) == true){
            printf("[%s:SUCCESS]\n",cmd.c_str());
            
            string output_message = input_buffer;

            size_t bufsize = strlen(output_message.c_str());

            networkutil network;
            
            network.sendOverSocket(sock,input_buffer);

            printf("[%s:END]\n",cmd.c_str());
        }else{
            printf("[%s:ERROR]\n",cmd.c_str());
            printf("[%s:END]\n",cmd.c_str());
        }
    }else{
        printf("[%s:ERROR]\n",cmd.c_str());
        printf("[%s:END]\n",cmd.c_str());
    }
}
/*
    Send BROADCAST message to server. This will send the clients message to 
    all connected clients, unless they have this client blocked.

    Format:
    <BROADCAST> <MSG>
*/
void client::doBROADCAST(const char* input_buffer,string cmd){

    printf("[%s:SUCCESS]\n",cmd.c_str());
    size_t bufsize = strlen(input_buffer);

    networkutil network;
    network.sendOverSocket(sock,input_buffer);

    printf("[%s:END]\n",cmd.c_str());
}
/*
    Sends BLOCK Message to server
    Updates Local Blocked Client Map - blockedClients

    Format: <BLOCK> <IP>


*/
void client::doBLOCK(const char* input_buffer,string cmd){
    string line = input_buffer;
    string ip = line.substr(line.find(" ") + 1);
    util util;
    ip = util.stringClean(ip.c_str());

    // Check if ValidIP
    if(util.isValidIP(ip) == false){
        doERROR(cmd);
    }else if(blockedClients.find(ip) != blockedClients.end() || ipExistsLocally(ip) == false){
        doERROR(cmd);
    }else{
        printf("[%s:SUCCESS]\n",cmd.c_str());
        string message_type = "BLOCK ";
        string input = message_type + ip;
        size_t bufsize = strlen(input.c_str());

        networkutil network;
        network.sendOverSocket(sock,input.c_str());

        blockedClients[ip] = ip;
        printf("[%s:END]\n",cmd.c_str());
    }
}
/*
    Sends UNBLOCK Message to server
    Updates Local Blocked Client Map - blockedClients

    Format: 
    <UNBLOCK> <IP>

*/
void client::doUNBLOCK(const char* input_buffer,string cmd){
    string line = input_buffer;
    string ip = line.substr(line.find(" ") + 1);
    util util;
    ip = util.stringClean(ip.c_str());

    // Check if ValidIP
    if(util.isValidIP(ip) == false){
        doERROR(cmd);
    }else if(blockedClients.find(ip) == blockedClients.end() || ipExistsLocally(ip) == false){
        doERROR(cmd);
    }else{
        printf("[%s:SUCCESS]\n",cmd.c_str());
        
        string message_type = "UNBLOCK ";
        string input = message_type + ip;
        size_t bufsize = strlen(input.c_str());

        networkutil network;
        network.sendOverSocket(sock,input.c_str());

        blockedClients.erase(ip);
        printf("[%s:END]\n",cmd.c_str());
    }
}

void client::doIP(string cmd){
    printf("[%s:SUCCESS]\n",cmd.c_str());
    string m = "IP:" + getIPAddress();
    printf(m.c_str());
    printf("[%s:END]\n",cmd.c_str());
}
void client::doPORT(string cmd){
    printf("[%s:SUCCESS]\n",cmd.c_str());
    string output = "PORT:" + getListeningPort();    
    printf(output.c_str());
    printf("\n[%s:END]\n",cmd.c_str());
}
void client::doAUTHOR(string cmd){
    string AUTHOR = "I, mitchelt, have read and understood the course academic integrity policy.\n";
    printf("[%s:SUCCESS]\n",cmd.c_str());
    printf(AUTHOR.c_str());
    printf("[%s:END]\n",cmd.c_str());
}
void client::doEXIT(string cmd){
    printf("[%s:SUCCESS]\n",cmd.c_str());
    printf("[%s:END]\n",cmd.c_str());
    exit(0);  
}
void client::doERROR(string cmd){
    printf("[%s:ERROR]\n",cmd.c_str());
    printf("[%s:END]\n",cmd.c_str());
}










/* #################################################################################
   ################################### Get/Set Functions############################
   #################################################################################
*/
void client::setListeningPort(string s){
    this->listening_port_string = s;
}
string client::getListeningPort(){
    return this->listening_port_string;
}
void client::setLoggedIn(bool b){
    this->loggedin = b;
}
bool client::getLoggedIn(){
    return this->loggedin;
}
void client::setClientList(string s){
    this->clientList = s;
}
string client::getClientList(){
    return this->clientList;
}
void client::setClientIPVector(vector<string> ip){
    this->clientIPVector = ip;
}

vector<string> client::getClientIPVector(){
    return this->clientIPVector;
}









/* #################################################################################
   ################################### Helper Functions ############################
   #################################################################################
*/
string client::getIPAddress(){
    
        struct sockaddr_in a;
        struct sockaddr_in in;
        int s;
        char buf[1024];
    
        s = socket(AF_INET,SOCK_DGRAM,0);
    
        a.sin_family = AF_INET;
        a.sin_port = htons(53);
        a.sin_addr.s_addr = inet_addr("8.8.8.8");
    
        bind(s,(struct sockaddr*)&a,sizeof(a));
        if(connect(s,(struct sockaddr*)&a,sizeof(a)) == -1){
            printf("Unable to connect\n");
        }
    
        getsockname(s,(sockaddr*)&in,(socklen_t*)sizeof(in));
    
        string  ip = inet_ntop(AF_INET,&in.sin_addr.s_addr,buf,sizeof(buf));
        ip = ip + "\n";
    
        close(s);
    
        return ip;
        
    }
    bool client::ipExistsLocally(string ip){
        for(int i = 0; i < clientIPVector.size(); i++){
            if(clientIPVector[i] == ip){
                return true;
            }
        }
        return false;
    }
    
