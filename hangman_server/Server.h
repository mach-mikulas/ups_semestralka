//
// Created by machm on 11.12.2023.
//

#ifndef HANGMAN_SERVER_SERVER_H
#define HANGMAN_SERVER_SERVER_H

#include <string>
#include <cstdio>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <chrono>
#include <sys/time.h>

#include "Client.h"
#include "ClientManager.h"
#include "MessageHandler.h"
#include "Game.h"


class Server {
public:
    /**
     * used for server starting server
     * @param ip ip for the server
     * @param port port for listening
     * @return -1 error, 0 ok
     */
    int start(const char *ip, int port);

    int connectionHandlerer(timeval* timeout);

private:

    int server_socket, client_socket;
    static struct sockaddr_in server_address, client_address, peer_address;
    //struct timeval timeout{};

    void tempDisconnect(ClientManager *client_mngr, GameManager* game_mngr, Client *client);

    void disconnet(ClientManager *client_mngr, GameManager *game_mngr, Client *client);

    void pingAllOnline(ClientManager *client_mngr);
};


#endif //HANGMAN_SERVER_SERVER_H
