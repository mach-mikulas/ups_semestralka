//
// Created by machm on 12.12.2023.
//

#ifndef HANGMAN_SERVER_CLIENTMANAGER_H
#define HANGMAN_SERVER_CLIENTMANAGER_H

#include "Client.h"
#include <cstdio>
#include <map>
#include <vector>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

class Game;

class ClientManager {
public:

    /**
     * stored file_descriptor with new instance of Client inside of map
     * @param file_descriptor
     * @return -1 error, otherwise 0
     */
    int newConnection(int file_descriptor);

    /**
     * Returns pointer to the object of Client
     * @param file_descriptor
     * @return pointer when client exists
     */
    Client* getClient(int file_descriptor);
    /**
     * temporarily disconnects the client
     */
    void shortDisconnectClient(Client* client);
    /**
     * Disconnects client from the server
     */
    void disconnectClient(Client* client);

    /**
     * Logs in client
     * @param client client that is trying to log in
     * @param name client login name
     * @return 0 - client logs in
     * @return -1 - client with this name already exists and is online
     */
    int logClient(Client* client, const std::string& name);


    std::map<std::string,Client*> client_names;
    std::list<Client*> client_list;

private:
    std::map<int,Client*> client_sockets;

    void reconnectMsg(Client* client);


};


#endif //HANGMAN_SERVER_CLIENTMANAGER_H
