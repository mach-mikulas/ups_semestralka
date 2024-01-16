//
// Created by machm on 29.12.2023.
//

#ifndef HANGMAN_SERVER_CONNECTIONSTATE_H
#define HANGMAN_SERVER_CONNECTIONSTATE_H

/**
 * @enum ConnectionState
 * @brief Describes the possible states of the client connection.
 *
 * This enum represents different states that the client connection can be in.
 * Each state has a specific meaning and is used to control the flow of the program.
 */
enum class ConnectionState {

    /** Client is connected */
    ONLINE,
    /** Client is offline for short period */
    SHORT_OFFLNE,
    /** Client is offline for a long time */
    LONG_OFFLINE
};

#endif //HANGMAN_SERVER_CONNECTIONSTATE_H
