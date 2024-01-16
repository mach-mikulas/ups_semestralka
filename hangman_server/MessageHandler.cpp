//
// Created by machm on 12.12.2023.
//

#include "MessageHandler.h"

const char* clientStatesStrings[] = {
        "UNLOGED",
        "LOGGED",
        "LOBBY",
        "WAITING_FOR_TURN",
        "ON_TURN"
};

//CONSCRUCTOR
    MessageHandler::MessageHandler(ClientManager *client_mngr, GameManager* game_mngr) : client_manager(client_mngr), game_manager(game_mngr) {}


    int MessageHandler::handle_message(std::string msg, Client* client) {
        std::istringstream iss(msg);
        std::vector<std::string> tokens;

        // Split the string using the specified delimiter
        std::string token;
        while (std::getline(iss, token, '|')) {
            tokens.push_back(token);
        }

        printMsg(msg, client);

        //------------------------------------------------
        //LOGIN MSG
        //------------------------------------------------
        if(StringHandlerer::removeEndl(tokens.at(0)) == "LOGIN"){

            std::regex pattern("^[a-zA-Z]{1,15}$");
            //WRONG LOGIN CLIENT STATE
            if(client->getState() != ClientState::UNLOGED){
                wrongState(client);
                Responder::sendResponse(client->getSocket(), "ERROR|1|1");
            //WRONG LOGIN MSG FORMAT
            } else if(tokens.size() != 2){
                wrongMsg(client, "wrong LOGIN msg format!");
                Responder::sendResponse(client->getSocket(), "ERROR|1|2");
            //WRONG LOGIN NAME
            } else if(!checkNameValidity(tokens.at(1))){
                wrongMsg(client, "invalid LOGIN name!");
                Responder::sendResponse(client->getSocket(), "ERROR|1|3");
            //CORRECT LOGIN
            } else {
                int login_result = client_manager->logClient(client, tokens.at(1));
                Client* old_client = client;
                client = client_manager->client_names[tokens.at(1)];
                std::string login_msg = "ACCEPT|1|" + std::to_string(PING_PERIOD) + "|" + std::to_string(MISSED_PING_TO_FULL_DISCONNECT);

                switch (login_result) {

                    case 0:
                        std::cout << "[MessageHandler] " << StringHandlerer::removeEndl(tokens.at(1)) << " logged in" << std::endl;
                        Responder::sendResponse(client->getSocket(), login_msg);
                        break;
                    case 1:
                        if(client->getState() == ClientState::WAITING_FOR_TURN || client->getState() == ClientState::ON_TURN){
                            client->setConnectionState(ConnectionState::ONLINE);
                            std::string reconn_msg = "RECONNECT|GAME|";
                            reconn_msg.append(std::to_string(PING_PERIOD) + "|" + std::to_string(MISSED_PING_TO_FULL_DISCONNECT));
                            Responder::sendResponse(client->getSocket(), reconn_msg);
                            client->didPingedBack = true;
                            client->missedPings = 0;
                            std::string quessed_chars_msg = "QUESSED|" + std::to_string(client->getGame()->getWord().length());

                            for (const auto& pair : client->getGame()->quessed_chars) {
                                char character = pair.first;               // Get the key
                                const std::vector<size_t>& values = pair.second;  // Get the associated vector

                                quessed_chars_msg.append("|");
                                quessed_chars_msg += character;

                                for (size_t value : values) {
                                    quessed_chars_msg.append( ";" + std::to_string(value));
                                }
                            }

                            Responder::sendResponse(client->getSocket(),quessed_chars_msg);
                            game_manager->gameUpdate(client->getGame());
                        } else if(client->getState() == ClientState::LOBBY){
                            client->setConnectionState(ConnectionState::ONLINE);
                            std::string reconn_msg = "RECONNECT|LOBBY|";
                            client->didPingedBack = true;
                            client->missedPings = 0;
                            reconn_msg.append(std::to_string(PING_PERIOD) + "|" + std::to_string(MISSED_PING_TO_FULL_DISCONNECT));
                            Responder::sendResponse(client->getSocket(), reconn_msg);
                            game_manager->lobbyUpdate(client->getGame());
                        }
                        break;
                    case -1:
                        std::cout << "[MessageHandler] " << " Client with name: " << StringHandlerer::removeEndl(tokens.at(1)) << ", already exists and is online!" << std::endl;
                        Responder::sendResponse(old_client->getSocket(), "ERROR|1|4");
                        return -1;
                        break;


                }
            }

            //TODO
            //CHECK FOR NAME ALREADY LOGGED
            //TODO

            return 1;

        //------------------------------------------------
        // JOIN MSG
        //------------------------------------------------
        } else if(StringHandlerer::removeEndl(tokens.at(0)) == "JOIN"){
            //WRONG JOIN CLIENT STATE
            if(client->getState() != ClientState::LOGGED){
                wrongState(client);
                Responder::sendResponse(client->getSocket(), "ERROR|2|1");
            //WRONG JOIN MSG FORMAT
            } else if(tokens.size() != 1){
                wrongMsg(client , "wrong JOIN msg format!");
                Responder::sendResponse(client->getSocket(), "ERROR|2|2");
            //CORRECT JOIN MSG
            } else {
                game_manager->joinGame(client);
                game_manager->lobbyUpdate(client->getGame());
                //START GAME IF THERE ARE 4 CLINETS IN
                //if(client->getGame()->getNumberOfClients() == 4){
                //    game_manager->startGame(client->getGame());
                //}

            }

        //------------------------------------------------
        // START MSG
        //------------------------------------------------
        } else if(StringHandlerer::removeEndl(tokens.at(0)) == "START"){

            //WRONG START CLIENT STATE
            if(client->getState() != ClientState::LOBBY){
                wrongState(client);
                Responder::sendResponse(client->getSocket(), "ERROR|3|1");
            //WRONG START MSG FORMAT
            } else if(tokens.size() != 1){
                wrongMsg(client , "wrong START msg format!");
                Responder::sendResponse(client->getSocket(), "ERROR|3|2");
            //LOW NUMBER OF CLIENT IN LOBBY
            } else if(client->getGame()->getNumberOfClients() < 2){
                wrongMsg(client , "low number of clients in lobby!");
                Responder::sendResponse(client->getSocket(), "ERROR|3|3");
            //CORRECT START MSG FORMAT
            } else {
                game_manager->startGame(client->getGame());
            }

        //------------------------------------------------
        // QUESS MSG
        //------------------------------------------------
        } else if(StringHandlerer::removeEndl(tokens.at(0)) == "QUESS"){

            std::regex pattern("[A-Z]");

            //WRONG QUESS CLIENT STATE
            if(client->getState() != ClientState::ON_TURN){
                MessageHandler::wrongState(client);
                Responder::sendResponse(client->getSocket(), "ERROR|4|1");
            }

            //WRONG QUESS MSG FORMAT
            else if(tokens.size() != 2 || !std::regex_match(StringHandlerer::removeEndl(tokens.at(1)), pattern)){
                wrongMsg(client , "wrong QUESS msg format!");
                Responder::sendResponse(client->getSocket(), "ERROR|4|2");
            //CORRECT QUESS MSG FORMAT
            } else {
                game_manager->quess(client, tokens.at(1).at(0));
                //TODO CHECK QUESS
            }



        } else if(StringHandlerer::removeEndl(tokens.at(0)) == "PONG"){
            client->didPingedBack = true;
        } else {
            wrongMsg(client,"Wrong msg from client");
        }



        return 0;
    }

    void MessageHandler::wrongMsg(Client *client, std::string errorMsg) {

    std::string s = client->getName();

    if(s.empty()){
        s = "UNLOGGED_CLIENT";
    }

    std::cout << "[" + StringHandlerer::removeEndl(s) + "] "<< StringHandlerer::removeEndl(errorMsg) << std::endl;
    client->incWrongMsg();
}

    void MessageHandler::wrongState(Client* client){
        std::string wrong_msg = "wrong client state! current state: ";
        wrong_msg.append(clientStatesStrings[static_cast<int>(client->getState())]);
        MessageHandler::wrongMsg(client , wrong_msg);
    }

void MessageHandler::printMsg(std::string &msg, Client *client) {

    if(client->getName().empty()){
        std::cout << "[UNLOGGED CLIENT] ";
    } else{
        std::cout << "[" + StringHandlerer::removeEndl(client->getName()) + "] ";
    }
    std::cout << msg << std::endl;

}

bool MessageHandler::checkNameValidity(const std::string& str) {

    int name_lenght = 0;

    std::string name = StringHandlerer::removeEndl(str);

    if(name.length() == 0){
        return false;
    }

    for (char c : name) {
        name_lenght++;
        if (!std::isalpha(c) || name_lenght > NAME_LENGHT) {
            return false;  // Return false if a non-alphabetical character is found or name is too long
        }
    }
    return true;  // Return true if all characters are alphabetical and name is shorter than NAME_LENGHT
}
