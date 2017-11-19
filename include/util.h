#ifndef UTIL_H_
#define UTIL_H_

#include <string.h>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <stdexcept>


#include "../include/server.h"

using namespace std;

class util{
private:

public:
    util();
    vector<string> stringTokenize(const char* input);
    string stringClean(const char* s);
    string toString(int n);
    bool isValidIP(string s);
    bool isValidPort(string p);
    bool ipExists(string s);
    bool isSingleCommand(const char* msg);
    string buildMessage(vector<string> s);
    vector<string> parseMessage(const char* msg);
    //void sendOverSocket(int sock,string input);
    //string parseFromSocket(const char* buffer);
 
};


#endif

