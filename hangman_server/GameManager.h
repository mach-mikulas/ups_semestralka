//
// Created by machm on 18.12.2023.
//

#ifndef HANGMAN_SERVER_GAMEMANAGER_H
#define HANGMAN_SERVER_GAMEMANAGER_H


#include <map>
#include <list>
#include "Game.h"
#include "StringHandlerer.h"

class GameManager {
public:

    GameManager();

    /** Connects client to the game that is not started and has free space*/
    void joinGame(Client* client);
    /** Starts the game*/
    void startGame(Game* game);
    /** Procceses clients quess*/
    void quess(Client *client, char quess_char);
    /** Sends gameUpdate msg to every client in the game*/
    void gameUpdate(Game* game);
    /** Sends lobbyUpdate msg to every client in the game*/
    void lobbyUpdate(Game* game);

    void announceWinner(Client* client);

    std::list<Game*> not_started_games;
    std::list<Game*> not_started_games_full;
    std::list<Game*> started_games;
private:

    void addClient(Client* client, Game* game);

    int number_of_created_games;
};


#endif //HANGMAN_SERVER_GAMEMANAGER_H
