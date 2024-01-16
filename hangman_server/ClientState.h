//
// Created by machm on 14.12.2023.
//

#ifndef HANGMAN_SERVER_CLIENTSTATE_H
#define HANGMAN_SERVER_CLIENTSTATE_H

/**
 * @enum ClientState
 * @brief Describes the possible states of the client.
 *
 * This enum represents different states that the client application can be in.
 * Each state has a specific meaning and is used to control the flow of the program.
 */
enum class ClientState {
    /** Client does not have name */
    UNLOGED,
    /** Client is connected, has name but he is not waiting in lobby */
    LOGGED,
    /** Client is inside of lobby and is waiting for the game to start */
    LOBBY,
    /** Client is inside of a game and is waiting for his turn */
    WAITING_FOR_TURN,
    /** Client is inside of a game and is on turn */
    ON_TURN
};


#endif //HANGMAN_SERVER_CLIENTSTATE_H
