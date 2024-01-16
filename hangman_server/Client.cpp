//
// Created by machm on 12.12.2023.
//

#include "Client.h"
#include <utility>


Client::Client(int socket) : socket(socket), client_state(ClientState::UNLOGED), client_connection(ConnectionState::ONLINE), game_id(-1), wrong_msg_counter(0), didPingedBack(true), missedPings(0){}

void Client::incWrongMsg() {
    wrong_msg_counter++;
}

void Client::initHP() {this->hp = STARTING_HP;}

void Client::decHP() {hp--;}

//SETTERS
void Client::setName(std::string new_name) {
    this->name = std::move(new_name);
}
void Client::setGame(Game* game) {
    this->game = game;
}
void Client::setState(ClientState new_state) {
    this->client_state = new_state;
}
void Client::setConnectionState(ConnectionState conn_state) {
    this->client_connection = conn_state;
}
void Client::setSocket(int fd) {
    this->socket = fd;
}


//GETTERS
ClientState Client::getState() {return client_state;}

std::string Client::getName() {return this->name;}

int Client::getSocket() {return this->socket;}

Game *Client::getGame() {return game;}

int Client::getHP() {return this->hp;}

ConnectionState Client::getConnectionState() {return client_connection;}

int Client::getWrongMsgCount() {return wrong_msg_counter;}





