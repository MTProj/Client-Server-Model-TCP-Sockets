#include "../include/server.h"
#include "../include/logger.h"
#include "../include/networkutil.h"

server::server()
{
}

//Public
void server::startServer(char p[]){

    maxclients = 100;
    port = atoi(p);

    /*
        Initialize server address struct
    */
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    inputsock = fileno(stdin);
    
    /*
        Create server socket. This socket will be used to listen
        and accept new connections.

    */
    server_socket = socket(AF_INET,SOCK_STREAM,0);
    if(server_socket < 0){
        printf("Unable to create socket \n");
    }

    /*
        Bind server_socket to port designated by passed in argument , p.
    */
    if(bind(server_socket, (struct sockaddr *)&server_addr,sizeof(server_addr)) <0){
        printf("Unable to bind socket to port \n");
        exit(EXIT_FAILURE);
    }

    /*
        Listen for incoming connections.
        server_socket
    */
    if(listen(server_socket,0) < 0){
        printf("Unable to listen on port \n");
    }

    int max_master_fd; // max file descriptor for master sockets explained in more detail below
    int max_client_fd; // max file descriptor for client sockets explained in more detail below
    int ready;
    caddr_len = sizeof(client_addr);
    inputsock = fileno(stdin);
    string cmd;
    vector<string> tokenized;
    util util;

    struct addrinfo caddrinfo;

    

    while(true){
        /*
            Initialize FD_SETs and find max fd for both sets
        */
        FD_ZERO(&master_list);
        FD_SET(server_socket,&master_list);
        FD_SET(inputsock,&master_list);
        max_master_fd = server_socket;
        if(inputsock > max_master_fd){ max_master_fd = inputsock; }
        
        for(int i = 0; i < clients_sockets.size(); i ++){
            FD_SET(clients_sockets[i].first,&master_list);
            if(clients_sockets[i].first > max_master_fd){
                max_master_fd = clients_sockets[i].first;
            }
        }

        /*
                Wait for Activity on Set of File Descriptors
        */
        ready = select(max_master_fd + 1,&master_list,NULL,NULL,NULL);
        if(ready < 0){ 
            printf("Error with select()\n");
        }else if(ready > 0){
            /* 
                Listening Socket is active - A new client is connecting
            */
            if(FD_ISSET(server_socket,&master_list)){

                /* Save Socket */
                client_socket = accept(server_socket,(struct sockaddr *)&client_addr,(socklen_t*)&caddr_len);
                
                /* Get Client IP */
                string ip = inet_ntoa(client_addr.sin_addr);
                int p = ntohs(client_addr.sin_port);


                /* Get client hostname */
                struct hostent *hostname;
                struct in_addr ipaddr;
                inet_pton(AF_INET,inet_ntoa(client_addr.sin_addr),&ipaddr);
                hostname = gethostbyaddr(&ipaddr,sizeof ipaddr,AF_INET);
                string hostName = hostname->h_name;

                /* Add Client */
                addClient(client_socket,ip,p,hostName);
            /*
                STDIN is active
            */
            }else if(FD_ISSET(inputsock,&master_list)){

                    //inval = read(inputsock,input_buffer,strlen(input_buffer));
                    int bufsize = 1025;
                    char* input_buffer = new char[bufsize];


                    inval = read(inputsock,input_buffer,bufsize);

                    int i = 0;
                    string m = "";
                    while(input_buffer[i] != '\n'){
                        m = m + input_buffer[i];
                        i++;
                    }

                    
                    
                    string temp = m;
                    //cout << "m=" << m << "\n";

                    if(inval < 0 || inval == 0){
                        printf("Error reading from socket\n");                        
                    }else{
                        // Check if command is single command or has other entries
                        if(util.isSingleCommand(input_buffer)){
                            cmd = util.stringClean(input_buffer);
                        }else{
                            tokenized = util.stringTokenize(input_buffer);
                            cmd = tokenized[0];
                        }
                        // Find what command was sent
                        if(cmd == "AUTHOR"){
                            doAUTHOR(cmd);
                        }else if(cmd == "IP"){
                            doIP(cmd);
                        }else if(cmd == "PORT"){
                            doPORT(cmd);
                        }else if(cmd == "LIST"){
                            doLIST(cmd);
                        }else if(cmd == "STATISTICS"){
                            doSTATISTICS(cmd);
                        }else if(cmd == "BLOCKED"){
                            doBLOCKED(input_buffer,cmd);
                        }else{
                            doERROR(cmd);    
                        }
                }

                delete [ ] input_buffer;
            /*
                A clients socket is active
            */
            }else{

                // Loop through clients sockets to see which is active in FD_SET
                for(int i = 0; i < clients_sockets.size(); i++){
                    if(FD_ISSET(clients_sockets[i].first,&master_list)){

                        /* Get data of client active in clients_sockets */
                        pair<int,string> activeClient = clients_sockets[i];

                        /* Read data from socket */
                        networkutil network;
                        string left = clientMap[activeClient.second].getLeftOverMessage();
                        vector<string> results = network.readFromSocket(activeClient.first);

                        /* Check if client disconnected - readFromSocket() returns the vector with nothing in if it has seen 0 from read*/
                        if(results.size() < 2){
                            // Client exited. 
                            parseEXIT(activeClient);
                            FD_CLR(activeClient.first,&master_list);
                        }else{

                            string _msg = results[0];

                            /* Parse Message */
                            string temp = network.parseFromSocket(_msg); 

                            if(util.isSingleCommand(temp.c_str())){
                                cmd = util.stringClean(temp.c_str());
                            }else{
                                tokenized = util.stringTokenize(temp.c_str());
                                cmd = tokenized[0];
                            }
                            if(cmd == "REFRESH"){
                                parseREFRESH(activeClient);
                            }else if(cmd == "LOGOUT"){
                                parseLOGOUT(activeClient);
                                FD_CLR(activeClient.first,&master_list);
                            }else if(cmd == "SEND"){
                                parseSEND(activeClient,temp.c_str());
                            }else if(cmd == "BROADCAST"){
                                parseBROADCAST(activeClient,temp.c_str());
                            }else if(cmd == "BLOCK"){
                                parseBLOCK(activeClient,temp.c_str());
                            }else if(cmd == "UNBLOCK"){
                                parseUNBLOCK(activeClient,temp.c_str());
                            }else if(cmd == "NEWCLIENT"){
                                parseNEWCLIENT(activeClient,temp.c_str());
                                sendListAndIPs(activeClient.first);
                            }else{
                                doERROR(cmd);
                            }
                        }
                        
                    }
                }
            }

        }

    }
}



  /* Do Functions */
void server::doAUTHOR(string cmd){
    const char* AUTHOR = "I, mitchelt, have read and understood the course academic integrity policy.\n";    
    printf("[%s:SUCCESS]\n",cmd.c_str());
    printf(AUTHOR);
    printf("[%s:END]\n",cmd.c_str());
}
void server::doIP(string cmd){
    printf("[%s:SUCCESS]\n",cmd.c_str());
    string m = "IP:" + getIPAddress();
    printf(m.c_str());
    printf("[%s:END]\n",cmd.c_str());
}
void server::doPORT(string cmd){
    util util;
    printf("[%s:SUCCESS]\n",cmd.c_str());
    string s2 = "\n";
    string s1 = util.toString(port);
    string s3 = "PORT:";
    string output = s3 + s1 + s2;

    printf(output.c_str());
    printf("[%s:END]\n",cmd.c_str());
}
void server::doLIST(string cmd){
    printf("[%s:SUCCESS]\n",cmd.c_str());
    printf(getListOfClients().c_str());
    printf("[%s:END]\n",cmd.c_str());
}
void server::doSTATISTICS(string cmd){
    printf("[%s:SUCCESS]\n",cmd.c_str());
    printf(getStatistics().c_str());
    printf("[%s:END]\n",cmd.c_str());
}
/*
    Message Format:<BLOCKED> <IP>
*/
void server::doBLOCKED(const char* buf,string cmd){
    string msg = buf;
    string ip = msg.substr(msg.find(" ") + 1);
    util util;

    if(util.isValidIP(ip) == true && ipExists(ip) == true){
        printf("[%s:SUCCESS]\n",cmd.c_str());    
        printf(getListOfBlockedClients(ip).c_str());
        printf("[%s:END]\n",cmd.c_str());
    }else{
        doERROR(cmd);
    }    
}

/*
Prints / Logs Error

*/
void server::doERROR(string cmd){
    printf("[%s:ERROR]\n",cmd.c_str());
    printf("[%s:END]\n",cmd.c_str());
}


 /* Server Parsing/Do Fucntions */

/*
    Client Sent LOGOUT notification. Logclient out but save state
    of information
*/
void server::parseLOGOUT(pair<int,string> activeClient){
    logClientOut(activeClient.first);
}

/*
    Client has exited. Log client out and remove from system.
*/
void server::parseEXIT(pair<int,string> activeClient){
    logClientOut(activeClient.first);
    clientMap.erase(activeClient.second);
}

/*
    Incoming Message Format: <SEND> <DESTINATION IP> <MSG>
*/
void server::parseSEND(pair<int,string> activeClient,string b){
    //cout << "parseSEND()\n";
    string fromIP = activeClient.second;
    string toIP = "";
    
    // Remove send
    string input_buffer = b;
    string buf = input_buffer.substr(input_buffer.find(" ") + 1);
    
    // Get Destination IP
    toIP = buf.substr(0,buf.find(" "));

    // Build Message
    string msg = buf.substr(buf.find(" ") + 1);
    string message_type = "NEWMESSAGE ";
    string sender = fromIP + " ";
    string input_msg = message_type + sender + msg;

    clientMap[fromIP].incMsgsSent();
    if(ipExists(toIP)){
        //cout << "IP exists\n";
        if(clientMap[toIP].isClientBlocked(fromIP) == false && clientMap[toIP].getOnlineStatus() == true){
            int sock = clientMap[toIP].getSocket();

            /* Send to client */
            networkutil network;
            network.sendOverSocket(sock,input_msg);
            clientMap[toIP].incMsgsRecvd();
            
            // Log Message On Server
            string logmessage = "msg from:" + fromIP + "\n" + "[msg]:" + msg;
            string command = "RELAYED";
            printf("[%s:SUCCESS]\n",command.c_str());
            printf(logmessage.c_str());
            printf("\n[%s:END]\n",command.c_str());            

        }else if(clientMap[toIP].isClientBlocked(fromIP) == false && clientMap[toIP].getOnlineStatus() == false){
            

            //cout << "Storing Message\n";
            clientMap[toIP].addMessages(input_msg);

            // Log Message On Server
            string logmessage = "msg from:" + fromIP + "\n" + "[msg]:" + msg;
            string command = "RELAYED";
            printf("[%s:SUCCESS]\n",command.c_str());
            printf(logmessage.c_str());
            printf("\n[%s:END]\n",command.c_str());
        }else{
            // Do Nothing - Client is blocked.
        }
    }else{
        //Do Nothing
    }    
}
/*
    Client Sends Broadcast Message to Server
    Server passes this message along to all clients who 
    do not have this client blocked

    Format: 
    <BROADCAST> <msg>
*/
void server::parseBROADCAST(pair<int,string> activeClient,const char* b){

    string fromIP = activeClient.second;
    string broadcast_ip = "255.255.255.255";
    
    // Remove Broadcast
    string input_buffer = b;
    string msg = input_buffer.substr(input_buffer.find(" ") + 1);

    // Build Message
    string message_type = "NEWMESSAGE ";
    string sender = fromIP + " ";
    string input_msg = message_type + sender + msg;

    // Increment messages sent for client who performed the broadcast. 
    // Consider this only one message?
    clientMap[fromIP].incMsgsSent();

    // Log the broadcast message
    string logmessage = "msg from:" + activeClient.second + ", to:" + broadcast_ip + "\n" + "[msg]:" + msg;
    string command = "RELAYED";
    printf("[%s:SUCCESS]\n",command.c_str());
    printf(logmessage.c_str());
    printf("\n[%s:END]\n",command.c_str());



    for(int i = 0; i < clients_sockets.size(); i++){

        string toIP = clients_sockets[i].second;

        if(ipExists(toIP) && toIP != fromIP){

            if(clientMap[toIP].isClientBlocked(fromIP) == false && clientMap[toIP].getOnlineStatus() == true){
               
                /*Client is online and does not have the sender blocked */

                int sock = clientMap[toIP].getSocket();
                size_t msgsize = strlen(input_msg.c_str());
                networkutil network;
                network.sendOverSocket(sock,input_msg.c_str());
                clientMap[toIP].incMsgsRecvd();
                
            }else if(clientMap[toIP].isClientBlocked(fromIP) == false && clientMap[toIP].getOnlineStatus() == false){
                
                /* Client is offline and does not have the sender blocked */
                clientMap[toIP].addMessages(input_msg);

            }else{
                /* Client has the sender blocked. Do not forward the message */
            }
        }else{
            /* IP address does not exist */
        }
    }
}
void server::parseBLOCK(pair<int,string> activeClient,const char* input_buffer){

    string from_ip = activeClient.second;
    string ip_to_block = "";
    util util;

    string input = input_buffer;
    ip_to_block = input.substr(input.find(" ") + 1);

    clientMap[from_ip].addBlockedClient(ip_to_block);

}
void server::parseUNBLOCK(pair<int,string> activeClient,const char* input_buffer){
    string from_ip = activeClient.second;
    string ip_to_unblock = "";
    util util;

    string input = input_buffer;
    ip_to_unblock = input.substr(input.find(" ") + 1);
    clientMap[from_ip].removeBlockedClient(ip_to_unblock);
}
void server::parseREFRESH(pair<int,string> activeClient){

    sendListAndIPs(activeClient.first);

}
void server::parseNEWCLIENT(pair<int,string> activeClient,const char* input_buffer){
    util util;

    // Save client listening port
    vector<string> parsed = util.parseMessage(input_buffer);
    string msg = parsed[1];
    clientMap[activeClient.second].setPort(atoi(msg.c_str()));
}

/* Helper Functions */
void server::addClient(int clientsock,string ip,int port,string hostname){
    
    if(clientMap.find(ip) != clientMap.end()){


        /*
            Client has connected before, update information in map.
            Send all buffered messages to client
        */
       clientMap[ip].setOnlineStatus(true);
       clientMap[ip].setSocket(clientsock);
       
       // Send Backlogged Messages
       vector<string> msgs = clientMap[ip].getMessages(); 
       string message_type = "BACKLOGGED_MESSAGES ";
       string messages = "";
       for(int i = 0; i < msgs.size(); i++){
           messages = messages + msgs[i] + "\n";
           clientMap[ip].incMsgsRecvd();
       }
       messages = message_type + messages;
       size_t message_size = strlen(messages.c_str());
       networkutil network;
       network.sendOverSocket(clientMap[ip].getSocket(),messages);
    }else{
        /* 
            Brand New Client Connecting
        */

        clientobj client;
        client.setSocket(clientsock);
        client.setHostName(hostname);
        client.setIP(ip);
        client.setOnlineStatus(true);
        client.setLeftOverMessage("");

        addToClientMap(client);

    }   

    // Add Client Socket to socket array and sort by listening port
    pair<int,string> c = make_pair(clientsock,ip);
    clients_sockets.push_back(c);
}
vector <pair<int,string> > server::selectionSortArray(vector<pair<int,string> > a){
    clientobj c1,c2;
    int minIndex;

    for(int i = 0; i < a.size()-1; i++){
        minIndex = i;
            for(int j = i+1; j < a.size(); j++){
                c1 = clientMap[a[j].second];
                c2 = clientMap[a[minIndex].second];
                if(c1.getPort() < c2.getPort()){
                    minIndex = j;
                }
            pair<int,string> temp;
            temp = a[minIndex];
            a[minIndex] = a[i];
            a[i] = temp;
        }
    }
    return a;
}

/*
    Used when a client logs out
    Removes the client socket from client_sockets array
    when a client logs back it will be added again.
*/
void server::logClientOut(int clientsock){

    for(int i = 0; i < clients_sockets.size(); i++){
        if(clients_sockets[i].first == clientsock){
            clientMap[clients_sockets[i].second].setOnlineStatus(false);
            close(clients_sockets[i].first);   
            clients_sockets.erase(clients_sockets.begin() + i);
            break;
        }
    }

}
void server::sendListAndIPs(int socket){
    //Send online client list to client
    string list = getListOfClients();
    string newlist = "NEWLIST ";
    newlist = newlist + list;

    string onlineIP = getOnlineIpAddresses();
    string newIPs = "CLIENTIP ";
    newIPs = newIPs + onlineIP;

    string input = newlist + newIPs;
    size_t inputSize = strlen(input.c_str());

    networkutil network;
    network.sendOverSocket(socket,input.c_str());
}

/*
    getListofClients(). Builds a string of all clients connected and returns. 
    Will have to print the string that is returned. 
*/
string server::getListOfClients(){

    util util;
    string line;
    vector <string> list;

    // Sort clients_sockets
    if(clients_sockets.size() > 1){
        clients_sockets = selectionSortArray(clients_sockets);
    }
    
    // Build list string from sorted clients sockets
    for(int d = 0; d < clients_sockets.size(); d++){
        line = "";
        clientMap[clients_sockets[d].second].setIdentifier(d+1);

        clientobj client = clientMap[clients_sockets[d].second];
        string ip = client.getIP();
        int port = client.getPort();
        int id = client.getIdentifier();
        string hostname = client.getHostName();
        
        line = util.toString(id) + "    " + hostname + "    " + ip + "    " + util.toString(port) + "\n";
        list.push_back(line);
    }
    

    // Build list ouput from vector strings
    string liststr = "";
    for(int i = 0; i < list.size(); i++){
        liststr = liststr + list[i] ;
    }
    
    return liststr;

}
/*
    Build List of Statistics
*/
string server::getStatistics(){
    util util;
    string line;
    vector <string> list;
    vector<pair<int,string> > allClients;

    // Add all clients in clientMap to vector with their listening port and IP
    for(map<string,clientobj>::iterator it = clientMap.begin(); it != clientMap.end(); it++){
        allClients.push_back(make_pair(it->second.getPort(),it->second.getIP()));
    }

    // If no clients just return empty string.
    if(allClients.size() == 0){
        return "";
    }

    // If less than 2 clients do not bother sorting
    if(allClients.size() < 2){
    
    }else{
        allClients = selectionSortArray(allClients);
    }

    // Build Statistics String
    for(int d = 0; d < allClients.size(); d++){
        line = "";
        clientMap[allClients[d].second].setIdentifier(d+1);

        clientobj client = clientMap[allClients[d].second];
        bool status = client.getOnlineStatus();
        string msgrecv = util.toString(client.getMsgsRecvd());
        string msgsent = util.toString(client.getMsgsSent());
        string hostname = client.getHostName();
        string id = util.toString(client.getIdentifier());

        string c_status = "";
        if(status == true){
            c_status = "logged-in";
        }else{
            c_status = "logged-out";
        }
        
        line = id + "   " + hostname + "    " + msgsent + "    " + msgrecv + "    " + c_status + "\n";
        list.push_back(line);
    }
    
    //cout << "building from vector \n";
    // Build list ouput from vector of strings
    string liststr = "";
    for(int i = 0; i < list.size(); i++){
        liststr = liststr + list[i];
    }

    //cout << "Returning\n";
    return liststr;
}

string server::getOnlineIpAddresses(){
    string ips = "";
    for(int i = 0; i < clients_sockets.size(); i++){
        ips = ips + clients_sockets[i].second + " ";
    }
    return ips;
}

/*
    Build list of blocked clients
*/
string server::getListOfBlockedClients(string ip){

            util util;
            string line;
            vector<string> list;
            ip = util.stringClean(ip.c_str());

            
            if(clientMap.find(ip) == clientMap.end()){

            }else{

            }
            clientobj c = clientMap[ip];
            map<string,string> blocked_clients = c.getBlockedClients(); // Get IP's of blocked clients
            
            vector<pair<int,string> > allClients;
            for(map<string,string>::iterator it = blocked_clients.begin(); it!=blocked_clients.end();it++){
                string c_ip = it->first;
                clientobj c1 = clientMap[c_ip];

                int listening_port = c1.getPort();
                string ip = c1.getIP();
    
                allClients.push_back(make_pair(listening_port,ip));
            }       
    
            // Sort Clients
            if(allClients.size() < 2){
                
            }else{
                allClients = selectionSortArray(allClients);
            }
    
            // Build Blocked String
            for(int d = 0; d < allClients.size(); d++){
                line = "";
                clientMap[allClients[d].second].setIdentifier(d+1);
        
                clientobj client = clientMap[allClients[d].second];
                string ip = client.getIP();
                int port = client.getPort();
                int id = client.getIdentifier();
                string hostname = client.getHostName();
                
                line = util.toString(id) + "    " + hostname + "    " + ip + "    " + util.toString(port) + "\n";
                list.push_back(line);
            }
    
            // Build ouput from vector strings above
            string liststr = "";
            for(int i = 0; i < list.size(); i++){
               liststr = liststr + list[i];
            }

            return liststr;
}

void server::addToClientMap(clientobj client){
    this->clientMap.insert(make_pair(client.getIP(),client));
}
void server::removeFromClientMap(clientobj client){
    this->clientMap.erase(client.getIP());
}
string server::getIPAddress(){
    
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
bool server::ipExists(string ip){

    util util;
    ip = util.stringClean(ip.c_str());

    if(clientMap.find(ip) == clientMap.end()){
        // Not Found 
        return false;
    }else{

        return true;
    }
}

/* Private Class - clientobj */
server::clientobj::clientobj(){
    this->msgsrecvd = 0;
    this->msgssent = 0;

}
void server::clientobj::setSocket(int s){
    this->sock = s;
}
int server::clientobj::getSocket(){
    return this->sock;
}

void server::clientobj::setIP(string ipv4){
    this->ip = ipv4;
}
string server::clientobj::getIP(){
    return this->ip;
}
void server::clientobj::setPort(int p){
    this->port = p;
}
int server::clientobj::getPort(){
    return this->port;
}
void server::clientobj::setOnlineStatus(bool online){
    this->online = online;

}
bool server::clientobj::getOnlineStatus(){
    return this->online;
}
void server::clientobj::addMessages(string msg){
    this->storedMessages.push_back(msg);
}
vector<string> server::clientobj::getMessages(){
    return this->storedMessages;
}
void server::clientobj::setHostName(string s){
    this->hostname = s;
}
string server::clientobj::getHostName(){
    return this->hostname;
}
void server::clientobj::incMsgsRecvd(){
    this->msgsrecvd++;
}
int server::clientobj::getMsgsRecvd(){
    return this->msgsrecvd;
}
void server::clientobj::incMsgsSent(){
    this->msgssent++;

}
int server::clientobj::getMsgsSent(){
    return this->msgssent;
}
void server::clientobj::setIdentifier(int n){
    this->id = n;
}
int server::clientobj::getIdentifier(){
    return this->id;
}
void server::clientobj::addBlockedClient(string ip){
    this->blockedClients[ip] = ip;
}
void server::clientobj::removeBlockedClient(string ip){
    this->blockedClients.erase(ip);
}
map<string,string> server::clientobj::getBlockedClients(){
    return this->blockedClients;
}
bool server::clientobj::isClientBlocked(string ip){
    if(this->blockedClients.find(ip) == blockedClients.end()){
        return false;
    }else{
        return true;
    }
}
void server::clientobj::setLeftOverMessage(string s){
    this->left_over_msg = s;
}
string server::clientobj::getLeftOverMessage(){
    return this->left_over_msg;
}
