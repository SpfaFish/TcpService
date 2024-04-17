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
        std::string opt;
        std::cin >> opt;
        if(opt == "EOF") {
            break;
        }
        int res;
        if(opt == "int32") {
            int32_t x;
            std::cin >> x;
            tcp::Value value;
            value.set(x);
            res = client->send(value);
        } else if(opt == "int64") {
            int64_t x;
            std::cin >> x;
            tcp::Value value;
            value.set(x);
            value.output();
            res = client->send(value);
        } else if(opt == "string") {
            std::string x;
            std::cin >> x;
            tcp::Value value;
            value.set(x);
            res = client->send(value);
        } else if(opt == "string_list") {
            int n;
            std::cin >> n;
            std::vector<std::string> v;
            v.reserve(n);
            for(int i = 0; i < n; i++) {
                std::string x;
                std::cin >> x;
                v.emplace_back(std::move(x));
            }
            tcp::Value value;
            value.set(std::move(v));
            res = client->send(value);
        }
        // int res = client->send(data);
        if(res == -1) {
            std::cout << "send failed" << std::endl;
        } else if(res == 0) {
            std::cout << "send success" << std::endl;
        } else if(res == 1) {
            std::cout << "server closed" << std::endl;
        }
    }
    return 0;
}