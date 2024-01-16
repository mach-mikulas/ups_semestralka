//
// Created by machm on 18.12.2023.
//

#include "GameManager.h"

GameManager::GameManager() {
    number_of_created_games = 0;
    GameManager::not_started_games = {};
    GameManager::not_started_games_full = {};
    GameManager::started_games = {};
}

void GameManager::joinGame(Client* client){

    Game* joinable_game;

    //TODO

    if(not_started_games.empty()){
        //CREATE NEW GAME AND INSERT IN TO THE LIST
        Game* new_game = new Game(number_of_created_games);
        number_of_created_games++;
        not_started_games.push_back(new_game);
        joinable_game = new_game;

    } else {
        //Game that is not started and has free space exists
        joinable_game = not_started_games.front();

    }

    //ADD PLAYER TO THE GAME
    addClient(client, joinable_game);

    std::cout << "[GameManager] " << StringHandlerer::removeEndl(client->getName()) << " joined game " << joinable_game->getId() << std::endl;
    Responder::sendResponse(client->getSocket(), "ACCEPT|2");

}

void GameManager::addClient(Client *client, Game *game) {
    client->initHP();
    client->setGame(game);
    client->setState(ClientState::LOBBY);
    game->addClient(client);

    if(game->getNumberOfClients() == 4){
        not_started_games.remove(game);
        not_started_games_full.push_back(game);
    }
}

void GameManager::startGame(Game *game) {

    if(game->getNumberOfClients() == 4){
        not_started_games_full.remove(game);
    } else {
        not_started_games.remove(game);
    }

    started_games.push_back(game);
    game->gameStart();
}

void GameManager::quess(Client *client, char quess_char) {

    std::vector<size_t> char_indexes = client->getGame()->checkQuess(quess_char);

    //WRONG QUESS
    if(char_indexes.empty()){

        //DECRESS CLIENTS HP
        client->decHP();

        std::string bad_quess_msg = "BADQUESS|";
        //ADDS QUESSING_CLIENT_NAME|HIS HP| to the msg
        bad_quess_msg.append(StringHandlerer::removeEndl(client->getName()) + ";" + std::to_string(client->getHP()) + "|");
        client->setState(ClientState::WAITING_FOR_TURN);
        client->getGame()->nextClient();
        client->getGame()->getClientOnTurn()->setState(ClientState::ON_TURN);
        //ADDS NEXT CLIENT ON TURN to the msg
        bad_quess_msg.append(StringHandlerer::removeEndl(client->getGame()->getClientOnTurn()->getName()));
        //SENDS THE MSG TO EVERYONE IN THE SAME GAME
        for (Client* client_bad : client->getGame()->getClientList()) {
            Responder::sendResponse(client_bad->getSocket(), bad_quess_msg);
        }

        Client* last_alive = client->getGame()->lastAliveClient();

        if(last_alive != nullptr){

            announceWinner(last_alive);
            return;
        }

    //CHECK IF CLIENT ON TURN IS A WINNER
    } else if(client->getGame()->isClientWinner()) {

        announceWinner(client);

        return;
    //CORRECT QUESS
    } else {

        std::string correct_quess_msg = "CORRECTQUESS|";
        correct_quess_msg += quess_char;

        //ADDS ALL INDEXES OF THE QUESSES CHAR IN QUESSED WORD
        for (size_t index : char_indexes) {
            correct_quess_msg.append(";" + std::to_string(index));
        }

        //SENDS THE MSG TO EVERYONE IN THE SAME GAME
        for (Client* client_correct : client->getGame()->getClientList()) {
            Responder::sendResponse(client_correct->getSocket(), correct_quess_msg);
        }

    }

}

void GameManager::gameUpdate(Game *game) {

    std::string update_msg = "GAMEUPDATE";

    //BUILDS THE UPDATE MSG STRING
    for (Client* client : game->getClientList()) {
        std::string con_state = "0";
        if(client->getConnectionState() == ConnectionState::SHORT_OFFLNE){
            con_state = "1";
        }
        update_msg.append(StringHandlerer::removeEndl("|" + client->getName()) + ";" + std::to_string(client->getHP()) + ";" + con_state);
    }

    update_msg.append("!" + StringHandlerer::removeEndl(game->getClientOnTurn()->getName()));

    //SENDS THE UPDATE MSG TO EVERY CLIENT IN GAME
    for (Client* client : game->getClientList()) {
        std::string con_state = "0";
        Responder::sendResponse(client->getSocket(), update_msg);
    }

}

void GameManager::lobbyUpdate(Game *game) {
    std::string update_msg = "LOBBYUPDATE";

    //BUILDS THE UPDATE MSG STRING
    for (Client* client : game->getClientList()) {

        std::string con_state = "0";

        if(client->getConnectionState() == ConnectionState::SHORT_OFFLNE){
            con_state = "1";
        }
        update_msg.append(StringHandlerer::removeEndl("|" + client->getName()) + ";" + con_state);
    }

    //SENDS THE UPDATE MSG TO EVERY CLIENT IN GAME
    for (Client* client : game->getClientList()) {
        Responder::sendResponse(client->getSocket(), update_msg);
    }
}

void GameManager::announceWinner(Client* client){

    std::cout << "[GameManager] " << StringHandlerer::removeEndl(client->getName()) << " won in game: " << client->getGame()->getId() << std::endl;

    std::string winner_msg = "WINNER|";
    winner_msg.append(StringHandlerer::removeEndl(client->getName()) + "|" + StringHandlerer::removeEndl(client->getGame()->getWord()));

    started_games.remove(client->getGame());

    if(client->getGame()->getNumberOfClients() == 4){
        not_started_games_full.push_back(client->getGame());
    } else {
        not_started_games.push_back(client->getGame());
    }


    //SENDS THE MSG TO EVERYONE IN THE SAME GAME
    for (Client* client_winner : client->getGame()->getClientList()) {
        client_winner->setState(ClientState::LOBBY);
        client_winner->initHP();
        Responder::sendResponse(client_winner->getSocket(), winner_msg);
    }

    lobbyUpdate(client->getGame());

}


