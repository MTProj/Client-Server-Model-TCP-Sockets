#ifndef NETWORKUTIL_H_
#define NETWORKUTIL_H_

#include <string.h>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <stdexcept>

#include "../include/global.h"

using namespace std;

class networkutil{
    private:    

    public:
        string begin_msg;
        string end_msg;
        int begin_msg_size;
        int end_msg_size;

        char* input_buffer;
        int bufsize;

        networkutil();
        void sendOverSocket(int sock,string msg);
        vector<string> readFromSocket(int sock);
        string parseFromSocket(string s);
 
};


#endif