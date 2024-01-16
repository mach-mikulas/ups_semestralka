//
// Created by machm on 12.12.2023.
//

#ifndef HANGMAN_SERVER_MESSAGEHANDLER_H
#define HANGMAN_SERVER_MESSAGEHANDLER_H

#include <iostream>
#include <sstream>
#include <vector>
#include <regex>
#include "Client.h"
#include "ClientManager.h"
#include "GameManager.h"
#include "Responder.h"
#include "StringHandlerer.h"
#include "config.h"

class MessageHandler {

public:

    MessageHandler(ClientManager* client_mngr, GameManager* game_mngr);

    /**
     * used for validating and reacting to messages from clients
     * @param msg message from client
     * @return -1 error, 0 ok
     */
    int handle_message(std::string msg, Client* client);


    static void wrongMsg(Client* client, std::string errorMsg);

private:

    static bool checkNameValidity(const std::string& str);
    static void printMsg(std::string &msg, Client* client);
    static void wrongState(Client* client);
    ClientManager* client_manager;
    GameManager* game_manager;
};


#endif //HANGMAN_SERVER_MESSAGEHANDLER_H
