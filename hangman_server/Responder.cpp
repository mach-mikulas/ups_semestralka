//
// Created by machm on 21.12.2023.
//
#include "Responder.h"


void Responder::sendResponse(int fd, std::string msg) {

    msg = msg + "\n";

    send(fd, msg.data(), msg.length(), 0);
}

