#include "../include/networkutil.h"

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


networkutil::networkutil(){
    begin_msg = "<BEGIN_MSG>";
    end_msg = "<END_MSG>";

    end_msg_size = strlen(end_msg.c_str());
    begin_msg_size = strlen(begin_msg.c_str());

    bufsize = 1025;

}

/*
    Message Packaging
*/

using namespace std;
void networkutil::sendOverSocket(int sock, string s){

    /* Add BEGIN and END tag to message */
    string msg = begin_msg + s + end_msg;
    //cout << "sendOverSocket('" << msg << "')\n";

    /* Send entire message */
    int bytesSent;
    size_t msg_size = strlen(msg.c_str());
    bytesSent = send(sock,msg.c_str(),msg_size,0);

    /* If all bytes have not been sent. Continue sending until they have been */
    while(bytesSent < msg_size){ 
        string leftover = msg.substr(bytesSent);
        bytesSent = send(sock,leftover.c_str(),strlen(leftover.c_str()),0);
    }

    //cout << "done\n";
}

/*
    Might need to change this to return a vector
    [1] = whole message
    [2] = left over data

*/
vector<string> networkutil::readFromSocket(int sock){
    int bufsize = 1025;
    char* input_buffer = new char[bufsize];

    vector<string> readmsgs;
    string left_over_data = "";

    /*  Read data from socket. Read until a complete message is recognized.
        Any left over data will be saved in a string and returned for the next read 
    */

    int bytesRead;
    bytesRead = read(sock,input_buffer,bufsize);
    string msg = input_buffer;

    if(bytesRead == 0){
        return readmsgs;
    }else{

        while(msg.find(begin_msg) == -1 || msg.find(end_msg) == -1){
            bytesRead = read(sock,input_buffer,bufsize);
            if(bytesRead == 0){
                return readmsgs;
            }
            string newmsg = input_buffer;
            msg = msg + newmsg;
            //cout << "msg = " << msg << "\n";
        }
    
        /* Build whole message to return */
        msg = msg.substr(msg.find(begin_msg),msg.find(end_msg));
        /* Will need to return this as well. */
        left_over_data = msg.substr(msg.find(end_msg)+1);
        readmsgs.push_back(msg);
        readmsgs.push_back(left_over_data);
        return readmsgs;
    }

    delete [ ] input_buffer;


}
/*
    Message depackaging
    What should be passed in is a complete message only, with a begin and end
    this will remove the begin and end tags.
*/
string networkutil::parseFromSocket(string message){
    //cout << "pareFromSocket(" << message << ")\n";

    if(message.find(begin_msg) == -1){
        //cout << "Error Parsing. Begin Message or end now found\n";
        return "";
    }else{
        message = message.substr(0,message.find(end_msg));
        message = message.substr((message.find(begin_msg) + begin_msg_size));
        return message;
    }
}