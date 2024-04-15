#include <cstdio>
#include <iostream>
#include <string>

#include "../include/ResourceManager.hpp"

int main(int argv, char* args[]) {
    if(argv != 2) {
        return -1;
    }
    int port = std::stoi(args[1]);
    std::cout << "server port: " << port << std::endl;
    
    auto server = tcp::ResourceManager::getSimpleTcpService(port);

    server->run();
    return 0;
}