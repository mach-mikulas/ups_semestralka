//
// Created by machm on 18.12.2023.
//


#include "Game.h"
#include "config.h"

Game::Game(int id) {
    this->id = id;
    std::cout << "[Game: " << Game::id << "] created" << std::endl;
}

void Game::gameStart() {

    this->found_chars = 0;
    this->word = getRandomWord();
    this->client_on_turn = client_list.front();
    this->quessed_chars = std::map<char, std::vector<size_t>>();

    std::string start_msg = "STARTING";
    start_msg.append("|" + std::to_string(STARTING_HP) + "|" + std::to_string(this->word.length()));

    start_msg.append("|" + StringHandlerer::removeEndl(this->client_on_turn->getName()));

    std::cout << start_msg << std::endl;

    for (Client* client : client_list) {
        client->setState(ClientState::WAITING_FOR_TURN);
    }

    client_on_turn->setState(ClientState::ON_TURN);

    //SENDS STARTING MSG
    for (Client* client : client_list) {
        Responder::sendResponse(client->getSocket(), start_msg);
    }

    std::cout << "[Game: " << Game::id << "] started with word: " << StringHandlerer::removeEndl(word) << std::endl;
}

std::vector<size_t> Game::checkQuess(char quess_char) {
    std::vector<size_t> indexes;

    //This char was already found
    if(quessed_chars.count(quess_char) > 0){
        return indexes;
    }

    size_t pos = this->word.find(quess_char);
    while (pos != std::string::npos) {
        found_chars++;
        indexes.push_back(pos);
        pos = this->word.find(quess_char, pos + 1); // Start searching again after the last found position
    }

    //Saving quessed char and its indexes
    if(!indexes.empty()){
        Game::quessed_chars.insert(std::make_pair(quess_char, indexes));
    }

    return indexes;
}

std::string Game::getRandomWord() {
    std::vector<std::string> words = WORD_POOL;

    if (words.empty()) {
        return "";  // Return an empty string if the vector is empty
    }

    // Initialize random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, words.size() - 1);

    // Generate a random index
    int randomIndex = dist(gen);

    // Return the string at the random index
    return words[randomIndex];
}

int Game::getId(){
    return this->id;
}

void Game::addClient(Client* client){
   client_list.push_back(client);
}
void Game::removeClient(Client* client){
    std::cout << "[Game: " << std::to_string(id) << "] Client: " << StringHandlerer::removeEndl(client->getName()) << " left game" << std::endl;

    client_list.remove(client);

    if(client_list.empty()){
        return;
    }

    //IF CLIENT WAS ON TURN SELECT NEW CLIENT
    if(client->getState() == ClientState::ON_TURN){
        nextClient();

        client->getGame()->getClientOnTurn()->setState(ClientState::ON_TURN);
    }

    //LAST CLIENT INSIDE A LOBBY AFTER REMOVING A CLIENT
    if(client_list.size() == 1){
        client_on_turn = client_list.front();
    }
}

int Game::getNumberOfClients(){
    return (int)client_list.size();
}

Client *Game::getClientOnTurn() {return client_on_turn;}

std::list<Client*> Game::getClientList() {return client_list;}

void Game::nextClient() {
    Client* next_client;
    auto iterator = std::find(client_list.begin(), client_list.end(), client_on_turn);

    do{
        auto nextIt = std::next(iterator) != client_list.end() ? std::next(iterator) : client_list.begin();
        next_client = *nextIt;
        iterator = nextIt;
    } while (next_client->getHP() <= 0);

    client_on_turn = next_client;
    std::cout << "[Game: " << this->id << "] next player on turn: " << StringHandlerer::removeEndl(client_on_turn->getName()) << std::endl;
}

bool Game::isClientWinner() {
    if(found_chars == word.size()){
        return true;
    } else {
        return false;
    }
}

Client* Game::lastAliveClient(){

    Client* last_client_alive;
    int clients_alive = 0;

    for(Client* alive_client : client_list){
        if(alive_client->getHP() > 0){
            last_client_alive = alive_client;
            clients_alive++;
        }
    }

    if(clients_alive == 1){
        return last_client_alive;
    } else {
        return nullptr;
    }

}

std::string Game::getWord() {
    return this->word;
}




