//
// Created by machm on 12.12.2023.
//

#ifndef HANGMAN_SERVER_CLIENT_H
#define HANGMAN_SERVER_CLIENT_H

#include <string>
#include <cstdio>
#include "ClientState.h"
#include "ConnectionState.h"
#include "Game.h"
#include "config.h"

//Forward declaration
class Game;

class Client {
public:

    Client(int socket);

    /** sets clients new name*/
    void setName(std::string name);
    /** sets clients new state*/
    void setState(ClientState new_state);
    /** sets clients connection state*/
    void setConnectionState(ConnectionState conn_state);
    /** Retuns name of the client */
    std::string getName();
    /** Retuns current state of the client */
    ClientState getState();
    /** Retuns clients socket */
    int getSocket();
    /** Changes client socket */
    void setSocket(int fd);
    /** Retuns current game */
    Game* getGame();
    /** Returns current number of HP */
    int getHP();
    /** Returns count of wrong msg sent by client */
    int getWrongMsgCount();
    /** Returns connection state of client */
    ConnectionState getConnectionState();
    /** Increments counter of wrong messages
     * @return 0 ok, -1 number of wrong msgs is higher than MAX_WRONG_MSGS
     */
    void incWrongMsg();
    /** sets game in which client is curently in*/
    void setGame(Game* game);
    /** inits HP to STARTING_HP*/
    void initHP();
    /** Decreases HP by 1*/
    void decHP();
    /** if client pinged back value is true*/
    bool didPingedBack;
    /** how many times client did not ping back in row*/
    int missedPings;

private:
    /** name of the client */
    std::string name;
    /** client socket */
    int socket;
    /** id of the game */
    int game_id;
    /** Game client currently playing */
    Game* game;
    /** Number of wrong messages that client sent */
    int wrong_msg_counter;
    /** state of the client */
    ClientState client_state;
    /** state of the client connection*/
    ConnectionState client_connection;
    /** number of hp in current game */
    int hp;
};


#endif //HANGMAN_SERVER_CLIENT_H
