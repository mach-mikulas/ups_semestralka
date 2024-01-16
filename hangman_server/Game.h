//
// Created by machm on 18.12.2023.
//

#ifndef HANGMAN_SERVER_GAME_H
#define HANGMAN_SERVER_GAME_H

#include <random>
#include <vector>
#include <list>
#include <iostream>
#include <map>
#include <algorithm>
#include "Client.h"
#include "StringHandlerer.h"
#include "Responder.h"

//Forward declaration
class Client;

class Game {

public:

    Game(int id);

    /**
     * used for starting the game
     */
    void gameStart();

    /**
     * used for checking if quessed char is inside of word
     * @return vector of indexes of quessed char
     */
    std::vector<size_t> checkQuess(char quess_char);

    /**
     * Sets next client for the turn
     */
    void nextClient();

    /**
     * used for getting id of the game
     * @return id of the game
     */
    int getId();

    std::list<Client*> getClientList();

    Client* getClientOnTurn();

    /**
     * used for getting numbers of clients in this game
     * @return number of clients inside of game
     */
    int getNumberOfClients();

    std::string getWord();

    /** Adds client to the game */
    void addClient(Client* client);
    /** Removes client from the game */
    void removeClient(Client* client);
    /** Checks if last quess completed whole word */
    bool isClientWinner();

    Client* lastAliveClient();
    std::map<char, std::vector<size_t>> quessed_chars;

private:

    /**
     * used for choosing random word for the game
     * @return randomly choosed string
     */
    std::string getRandomWord();


    std::list<Client*> client_list;
    Client* client_on_turn{};
    std::string word;
    int id;
    int found_chars;


};


#endif //HANGMAN_SERVER_GAME_H
