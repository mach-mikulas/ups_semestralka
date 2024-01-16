//
// Created by machm on 21.12.2023.
//

#ifndef HANGMAN_SERVER_RESPONDER_H
#define HANGMAN_SERVER_RESPONDER_H


#include <string>
#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include "Client.h"

class Responder {

public:

    /** Sends approval msg to client */
    static void sendResponse(int fd, std::string msg);


    /** Sends error msg to client */
    static void sendError(int fd, std::string error);
};


#endif //HANGMAN_SERVER_RESPONDER_H
