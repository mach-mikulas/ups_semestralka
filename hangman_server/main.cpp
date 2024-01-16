#include <iostream>
#include <sys/time.h>
#include "Server.h"

int isValidPort(const char* port) {
    try {
        // Attempt to convert the port string to an integer
        int port_num = std::stoi(port);

        if(port_num > 0 && port_num <= 65535){
            return port_num;
        } else {
            return -1;
        }
    } catch (const std::invalid_argument& e) {
        // Catch exception if the conversion fails due to invalid argument
        std::cout << "Invalid port. Port must be a valid integer." << std::endl;
        return -1;
    } catch (const std::out_of_range& e) {
        // Catch exception if the conversion results in an out-of-range value
        std::cout << "Invalid port. Port is out of range." << std::endl;
        return -1;
    }
}

int main(int argc, const char *argv[]) {

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    if (argc < 3) {
        std::cout << "Missing parameters" << std::endl;
        return -1; // Return an error code
    }

    int port = isValidPort(argv[2]);

    if(port == -1){
        return -1;
    }


    Server server;

    int start_result = server.start(argv[1], port);

    if(start_result == -1){
        return -1;
    }

    std::cout << "[main] Server started. IP: " << argv[1] << " Port: " << port << std::endl;

    server.connectionHandlerer(&timeout);

    return 0;
}

