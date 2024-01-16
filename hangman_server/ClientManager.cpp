//
// Created by machm on 12.12.2023.
//

#include "ClientManager.h"

int ClientManager::newConnection(int file_descriptor) {

    Client* new_client = new Client(file_descriptor);
    ClientManager::client_sockets.insert(std::make_pair(file_descriptor, new_client));
    client_list.push_back(new_client);

    std::cout << "[ClientManager->newConnection] New client connected" << std::endl;

    return 0;
}

Client* ClientManager::getClient(int file_descriptor) {

    return client_sockets[file_descriptor];
}

int ClientManager::logClient(Client *client, const std::string& name) {

    //CHECK IF CLIENT WITH THIS NAME ALREADY EXISTS
    if (client_names.find(name) != client_names.end()) {
        if(client_names[name]->getConnectionState() == ConnectionState::ONLINE){
            //TODO DISCONNECT
            return -1;
        //CLIENT WITH THIS NAME EXISTS AND IS TEMPORARILY DISCONNECTED
        } else {
            //DELETE CLIENT OBJECT THAT IS NOT NEEDED
            int fd = client->getSocket();
            client_list.remove(client);
            client_sockets.erase(fd);
            delete client;
            client = nullptr;

            //INSERT BACK CLIENT THAT WAS TEMPORARILY DISCONNECTED
            client_names[name]->setSocket(fd);
            client_sockets.insert(std::make_pair(fd, client_names[name]));

            std::cout << "[ClientManager] " << StringHandlerer::removeEndl(name) << " reconnected" << std::endl;

            return 1;
        }
    //CLIENT WITH THIS NAME DOES NOT EXISTS
    } else {
        client->setName(name);
        client->setState(ClientState::LOGGED);
        client_names.insert(std::make_pair(name, client));
        return 0;
    }

    return -2;
}


void ClientManager::disconnectClient(Client *client) {

    std::string client_name = client->getName();
    if(client_name.empty()){
        client_name = "!UNLOGGED!";
    }

    std::cout << "[ClientManager] Client: " << StringHandlerer::removeEndl(client_name) << " disconnected" << std::endl;

//TODO
    switch (client->getState()) {

        case ClientState::UNLOGED:

            break;

        case ClientState::LOGGED:
            client_names.erase(client->getName());
            break;
        case ClientState::LOBBY:
            client->getGame()->removeClient(client);
            std::cout << "DISSCONECT ON LOBBY" << std::endl;
            client_names.erase(client->getName());
            break;
        case ClientState::ON_TURN:
            client->getGame()->removeClient(client);
            std::cout << "DISSCONECT ON TURN" << std::endl;
            client_names.erase(client->getName());
            break;
        case ClientState::WAITING_FOR_TURN:
            client->getGame()->removeClient(client);
            std::cout << "DISSCONECT ON WAITING" << std::endl;
            client_names.erase(client->getName());
            break;
    }

}

void ClientManager::shortDisconnectClient(Client *client) {
    std::string client_name = client->getName();
    if(client_name.empty()){
        client_name = "!UNLOGGED!";
    }
    std::cout << "[ClientManager] Client: " << StringHandlerer::removeEndl(client_name) << " is not reachable" << std::endl;
    client_sockets.erase(client->getSocket());
    client->setSocket(-1);
    client->setConnectionState(ConnectionState::SHORT_OFFLNE);
}

void ClientManager::reconnectMsg(Client *client) {

    std::string reconnect_msg = "RECONNECT|";

    switch (client->getState()) {
        case ClientState::LOGGED:
            reconnect_msg.append("1");
            break;
        case ClientState::LOBBY:

            reconnect_msg.append("2");

            for (Client* client_lobby : client->getGame()->getClientList()) {
                reconnect_msg.append(StringHandlerer::removeEndl("|" + client_lobby->getName()) + ";" + std::to_string(client_lobby->getHP()));
            }
            break;

        case ClientState::WAITING_FOR_TURN:
        case ClientState::ON_TURN:

            reconnect_msg.append("3");

            for (Client* client_lobby : client->getGame()->getClientList()) {
                reconnect_msg.append(StringHandlerer::removeEndl("|" + client_lobby->getName()) + ";" + std::to_string(client_lobby->getHP()));
            }

            reconnect_msg.append("!" + StringHandlerer::removeEndl(client->getGame()->getClientOnTurn()->getName()));

            break;
    }

    Responder::sendResponse(client->getSocket(), reconnect_msg);
}


