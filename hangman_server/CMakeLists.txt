cmake_minimum_required(VERSION 3.18.4)
project(hangman_server)

set(CMAKE_CXX_STANDARD 17)

add_executable(hangman_server main.cpp
        Server.cpp
        Server.h
        MessageHandler.cpp
        MessageHandler.h
        Client.cpp
        Client.h
        ClientManager.cpp
        ClientManager.h
        ClientState.h
        Game.cpp
        Game.h
        GameManager.cpp
        GameManager.h
        config.h
        Responder.cpp
        Responder.h
        StringHandlerer.cpp
        StringHandlerer.h
        ConnectionState.h)
