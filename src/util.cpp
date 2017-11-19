#include "../include/util.h"

util::util(){}


using namespace std;
vector<string> util::stringTokenize(const char* input){
    
    vector<string> tokenized;
    string command;

    for(int i = 0; i < strlen(input); i++){
        
        /*
            Once there is a new line character we can exit the for loop
            Plan to implement another statement to exit once there are more than 3 spaces
            depending on what data is actually needed.

                                    ****NOTE****
            The break is also there to remedy a situation where the input of cin is 
            passing in extra characters than what the user actually typed. I have not been
            able to fix this yet so this is sort of a hacky fix for now. 

            Ex. User unters AUTHOR first. AUTHOR is read fine, but if another word is entered, say PORT. Then
            what is actually read in is PORT \nR. 



        */
        if(input[i] == '\n'){
            tokenized.push_back(command);
            break;

        }else if(input[i] == ' '){
            tokenized.push_back(command);
            command = "";
        }else if(i == strlen(input)-1){
            command = command + input[i];
            tokenized.push_back(command);
        }else{
            command = command + input[i];
        }
    }

    /*
    for(int i = 0; i < tokenized.size(); i++){
        cout << i << ":" << tokenized[i] << "\n";
    }
    */
    
    return tokenized;
}
string util::stringClean(const char* input){
    string result = "";
    for(int i = 0; i < strlen(input); i++){

        /*                          ***NOTE****
        The break is there to remedy a situation where the input of cin is 
        passing in extra characters than what the user actually typed. I have not been
        able to fix this yet so this is sort of a hacky fix for now. 

        Ex. User enters AUTHOR first. AUTHOR is read fine, but if another word is entered, say PORT. Then
        what is actually read in is PORT \nR. 

        */
        if(input[i] == '\n'){
            //skip character
            break;
        }else{
            result = result + input[i];
        }
    }
    return result;
}
string util::toString(int n){
    std::stringstream s;
    s << n;
    return s.str();

}
bool util::isValidIP(string s){
    // Clean string - remove any new line chars

    s = stringClean(s.c_str());


    vector<string> tokenizedIP;
    int s_size = s.length();

    //First check if size of string is valid
    if(s_size < 7 || s_size > 15){
        return false;
    }

    // Go through entire string at each . and evaluate if valid
    string octet = "";
    for(int i = 0; i < s_size; i++){
        if(s[i] == '.'){
            // Check if length of octet is between 1 and 3
            if(octet.length() < 1 || octet.length() > 3){
                return false;
            }else{
                tokenizedIP.push_back(octet);
                octet = "";
            }
        }else if(i == s_size - 1){
            // Last character of string

            // Check if current character is a digit and between 1 and 255
            if(isdigit(s[i]) == false || (int)s[i] < 1 || (int)s[i] > 255){
                return false;
            }else{
                octet = octet + s[i];
            }
            // Check if length of octet is between 1 and 3
            if(octet.length() < 1 || octet.length() > 3){
                return false;
            }else{
                tokenizedIP.push_back(octet);
                octet = "";
            }
        }else{
            // Check if current character is a digit and between 1 and 255
            if(isdigit(s[i]) == false || (int)s[i] < 1 || (int)s[i] > 255){
                return false;
            }else{
                octet = octet + s[i];
            }
        }
    }
    // Verify that there are only 4 octets
    if(tokenizedIP.size() == 4){
        return true;
    }else{
        return false;
    }
}
bool util::isValidPort(string p){

    // Convert string p to integer
    //   if conversion fails catch exception and return false;
    size_t sz;
    try{
        int port = atoi(p.c_str());
        // If conversion occurs check that the port is between 1 and 65535
        if(port < 1 || port > 65535){
            return false;
        }else{
            return true;
        }
    }catch(const invalid_argument& ia){
        return false;
    }
}


bool util::ipExists(string s){
    return true;
}


/*
    Build Message For Sending - This will be used by the client when sending a message
    and by the server when forwarding
*/
string util::buildMessage(vector<string> s){

    // Skip s[0] because that is the command (SEND);
    // Skip s[1] because that is the IP
    string msg = "";
    string ip = "";

    for(int i = 2; i < s.size(); i++){
        msg = msg + s[i] + " ";
    }

    ip = s[1];
    string entiremsg = ip + msg + "\n";

    return entiremsg;
}

/*
    Parse recving message - This will be used by clients when recving messages and 
    servers when recvd.
*/
vector<string> util::parseMessage(const char *msg){
    
    util u;
    vector<string> tokenized = stringTokenize(msg);
    string cmd = tokenized[0];

    vector<string> parsedmessage;

    string newmsg = "";
    for(int i = 1; i < tokenized.size(); i++){
        newmsg = newmsg + tokenized[i] + " ";
       
    }

    parsedmessage.push_back(cmd);
    parsedmessage.push_back(newmsg);

    return parsedmessage;
}
bool util::isSingleCommand(const char *msg){
    // If a space is found return false
    for(int i = 0; i < strlen(msg); i++){
        if(msg[i] == ' '){
            return false;
        }
    }
    return true;
}