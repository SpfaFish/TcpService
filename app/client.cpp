#include <iostream>
#include <string>

#include "../include/ResourceManager.hpp"

int main(int argv, char* args[]) {
    if(argv != 3) {
        return -1;
    }
    std::string ip(args[1]);
    int port = std::stoi(args[2]);
    std::cout << "server ip: " << ip << " port: " << port << std::endl;
    
    auto client = tcp::ResourceManager::getSimpleTcpClient(ip, port);
    while(true) {
        std::string data;
        std::cin >> data;
        if(data != "EOF") {
            int res = client->send(data);
            if(res == -1) {
                std::cout << "send failed" << std::endl;
            } else if(res == 0) {
                std::cout << "send success" << std::endl;
            } else if(res == 1) {
                std::cout << "server closed" << std::endl;
            }
            sleep(3);
        }
    }
    return 0;
}