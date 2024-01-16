//
// Created by machm on 11.12.2023.
//

#include <threads.h>
#include "Server.h"



int return_value, fd, a2read, len_address;
struct sockaddr_in Server::client_address, Server::server_address;
fd_set client_socks, tests;


int Server::start(const char *ip, int port) {

    //Creating socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    //Check if socket is created
    if (server_socket < 0) {
        std::cout << "[Server->start] Could not create socket" << std::endl;
        return -1;
    }

    std::cout << "[Server->start] Socket created" << std::endl;


    //set port release
    int enable = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        std::cout << "[Server->start] SO_REUSEADDR disabled" << std::endl;
    }


    server_address.sin_family = AF_INET;

    in_addr_t inAddr = inet_addr(ip);
    if (inAddr == -1) {
        std::cout << "[Server->start] Invalid IP adddress" << std::endl;
        return -1;
    } else {
        server_address.sin_addr.s_addr = inAddr;
    }

    server_address.sin_port = htons(port);


    //Binding adress and port
    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        //print the error message
        std::cout << "[Server->start] Bind failed. Error" << std::endl;
        return -1;
    }

    std::cout << "[Server->start] Binding done" << std::endl;


    //Listen
    if (listen(server_socket, 5) < 0) {
        std::cout << "[Server->start] Listen error" << std::endl;
        return -1;
    }

    std::cout << "[Server->start] Server has been started" << std::endl;

    return 0;
}

int Server::connectionHandlerer(timeval* timeout) {

    //Clearing client_socks
    FD_ZERO( &client_socks );
    //Adding server_socket to client_socks
    FD_SET( server_socket, &client_socks );

    ClientManager client_manager;
    GameManager* game_manager = new GameManager();
    MessageHandler* msg_handler = new MessageHandler(&client_manager, game_manager);
    auto last_ping_time = std::chrono::steady_clock::now();

    while(true){

        char msg_buf[2048];
        tests = client_socks;

        if(select( FD_SETSIZE, &tests, ( fd_set *)0, ( fd_set *)0, timeout ) < 0){
            return EXIT_FAILURE;
        }

        for(fd = 3; fd < FD_SETSIZE; fd++){

            if(FD_ISSET(fd, &tests)){
                //Server socket -> new connection

                if (fd == server_socket){

                    std::cout << "SOMEONE JOINED" << std::endl;

                    client_socket = accept(server_socket, (struct sockaddr *) &client_address, (socklen_t *) &len_address);

                    //TODO
                    client_manager.newConnection(client_socket);
                    //TODO

                    FD_SET(client_socket, &client_socks);

                //Client socket -> handle message
                } else {

                    ioctl(fd, FIONREAD, &a2read);
                    Client* client = client_manager.getClient(fd);

                    if(client == nullptr){
                        continue;
                    }

                    if (a2read > 0){

                        //MSG IS TOO LONG
                        if(BUFFER_SIZE < a2read){
                            client->incWrongMsg();
                            if(client->getWrongMsgCount() >= MAX_WRONG_MSGS){
                                tempDisconnect(&client_manager, game_manager, client);
                                disconnet(&client_manager, game_manager,client);
                            }
                            continue;
                        }

                        memset(msg_buf, 0, 2048);
                        recv(fd, &msg_buf, 2048, 0);
                        std::string msg(msg_buf);

                        int handle_result = msg_handler->handle_message(msg, client);

                        client = client_manager.getClient(fd);
                        if(client == nullptr){
                            continue;
                        }

                        if(handle_result == -1){
                            tempDisconnect(&client_manager, game_manager,client);
                            disconnet(&client_manager, game_manager,client);
                            continue;
                        }


                        //IF CLIENT SENT TO MANY WRONG MSGS DISCONNECT HIM
                        if(client->getWrongMsgCount() >= MAX_WRONG_MSGS){
                            std::cout << client->getWrongMsgCount() << std::endl;
                            tempDisconnect(&client_manager, game_manager,client);
                            disconnet(&client_manager, game_manager,client);
                            continue;
                        }

                    //CLIENT temp disconnect
                    } else if(a2read == 0){

                        //IF CLIENT IS LOGGED OR UNLOGGED COMPLETLY DISCONNECT
                        if(client->getState() == ClientState::UNLOGED || client->getState() == ClientState::LOGGED){
                            tempDisconnect(&client_manager, game_manager,client);
                            disconnet(&client_manager, game_manager,client);
                        //IF CLIENT IS IN LOBBY OR IN A GAME ONLY TEMP DISCONNECT
                        } else {
                            tempDisconnect(&client_manager, game_manager,client);
                            //disconnet(&client_manager, game_manager,client);
                        }

                    }
                }

            }

        }

        //PING
        auto time_now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(time_now - last_ping_time).count();
        
        if(elapsed >= PING_PERIOD){

            for (Client* client_to_ping : client_manager.client_list) {


                //Client did ping back
                if (client_to_ping != nullptr){
                    if (client_to_ping->didPingedBack) {
                        client_to_ping->missedPings = 0;
                        //Client did not ping back
                    } else {
                        client_to_ping->missedPings += 1;
                        //Temporarly disconnects client
                        if (client_to_ping->missedPings == 1) {

                            if (client_to_ping->getConnectionState() != ConnectionState::SHORT_OFFLNE) {
                                if (client_to_ping->getState() == ClientState::UNLOGED ||
                                    client_to_ping->getState() == ClientState::LOGGED) {
                                    tempDisconnect(&client_manager, game_manager, client_to_ping);
                                    disconnet(&client_manager, game_manager, client_to_ping);
                                    break;
                                } else {
                                    tempDisconnect(&client_manager, game_manager, client_to_ping);
                                }
                            }

                            //Fully disconnects client
                        } else if (client_to_ping->missedPings >= MISSED_PING_TO_FULL_DISCONNECT) {
                            std::cout << "PING PROBLEMOS" << std::endl;
                            disconnet(&client_manager, game_manager, client_to_ping);
                            break;
                        }
                    }

                    client_to_ping->didPingedBack = false;

                    std::string ping_msg = "PING";

                    Responder::sendResponse(client_to_ping->getSocket(), ping_msg);
                }


            }

            last_ping_time = time_now;

        }


        //TODO PING




    }

}

void Server::disconnet(ClientManager* client_mngr, GameManager* game_mngr, Client *client) {

    std::cout << "DISCONNECTING" <<std::endl;

    Game* clients_game = client->getGame();

    if(client->getState() == ClientState::LOBBY){
        if(clients_game->getNumberOfClients() == 4) {
            game_mngr->not_started_games_full.remove(clients_game);
            game_mngr->not_started_games.push_back(clients_game);
        }
    }

    client_mngr->disconnectClient(client);

    //CLIENT IS DISCONNECTED FROM A LOBBY
    if(client->getState() == ClientState::LOBBY){
        game_mngr->lobbyUpdate(clients_game);



        if(clients_game != nullptr && clients_game->getNumberOfClients() == 0){
            game_mngr->not_started_games.remove(clients_game);
            std::cout << "[GameManager] lobby: " << clients_game->getId() << " has been deleted" << std::endl;

            delete clients_game;
            clients_game = nullptr;
        }
    //CLIENT IS DISCONNECTED FROM A GAME
    } else if(client->getState() == ClientState::WAITING_FOR_TURN || client->getState() == ClientState::ON_TURN){

        game_mngr->gameUpdate(clients_game);

        if(clients_game != nullptr && clients_game->getNumberOfClients() == 1){
            game_mngr->announceWinner(clients_game->getClientOnTurn());
        }

        if(clients_game != nullptr && clients_game->getNumberOfClients() == 0){
            std::cout << "started_games size:" << std::to_string(game_mngr->started_games.size()) << std::endl;
            game_mngr->started_games.remove(clients_game);
            std::cout << "[GameManager] game: " << clients_game->getId() << " has been deleted" << std::endl;
            delete clients_game;
            clients_game = nullptr;
        }
    }

    std::cout << "[Server] Client " << StringHandlerer::removeEndl(client->getName()) << " has been deleted" << std::endl;
    client_mngr->client_list.remove(client);
    delete client;
    client = nullptr;

}

void Server::tempDisconnect(ClientManager *client_mngr, GameManager* game_mngr, Client *client) {

    std::cout << "[Server] fd: " << std::to_string(client->getSocket()) << " closed" << std::endl;
    close(client->getSocket());
    FD_CLR(client->getSocket(), &client_socks);

    client_mngr->shortDisconnectClient(client);

    if(client->getState() == ClientState::LOBBY){
        game_mngr->lobbyUpdate(client->getGame());
    } else if (client->getState() == ClientState::ON_TURN || client->getState() == ClientState::WAITING_FOR_TURN){
        game_mngr->gameUpdate(client->getGame());
    }

}

void Server::pingAllOnline(ClientManager *client_mngr){


    std::string ping_msg = "PING";

    for (Client* client_to_ping : client_mngr->client_list) {

        if(client_to_ping->getConnectionState() == ConnectionState::ONLINE){
            client_to_ping->didPingedBack = false;
            Responder::sendResponse(client_to_ping->getSocket(), ping_msg);

        }

    }
}


