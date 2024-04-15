#include <asm-generic/socket.h>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <memory>
#include <cstdlib>
#include <cstdio>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace tcp {


class Service {
  private:
    int socket_fd, client_fd;
    sockaddr_in s_addr, c_addr;
    bool ok;
  public:
    Service() = delete;
    Service(int port) {
        socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(socket_fd == -1) {
            std::cerr << "socket create error" << std::endl;
            perror("socket");
            ok = false;
            return;
        }
        std::cout << "socket fd: " << socket_fd << std::endl;
        std::memset(&s_addr, 0, sizeof(s_addr));
        s_addr.sin_family = AF_INET;
        s_addr.sin_port = htons(port);
        // s_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        s_addr.sin_addr.s_addr = INADDR_ANY;
        int i = 1;
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));                
        int res = bind(socket_fd, (sockaddr*)&s_addr, sizeof(s_addr));
        if(res == -1) {
            std::cerr<< "bind error" << std::endl;
            perror("bind");
            ok = false;
            return;
        }
        std::cout << "bind code: " << res << std::endl;
        res = listen(socket_fd, 16);
        if(res == -1) {
            std::cerr << "listen error" << std::endl; 
            perror("listen");
            ok = false;
            return;
        }
        std::cout << "listen code: " << res << std::endl;
        std::cout << "init Service success" << std::endl;
        ok = true;
    }
    ~Service() {
        close(socket_fd);
        close(client_fd);
    }
    bool acceptConn() {
        if(!ok) {
            return false;
        }

        std::memset(&c_addr, 0, sizeof(c_addr));
        uint32_t client_addr_len = sizeof(c_addr);
        std::cout << "listen on the port..." << std::endl;
        client_fd = accept(socket_fd, (sockaddr*)(&c_addr), &client_addr_len);
        if(client_fd == -1) {
            std::cerr << "init client socket error" << std::endl;
            perror("accept");
            return false;
        }

        char client_ip[32];
        auto client_port = ntohs(c_addr.sin_port);
        inet_ntop(AF_INET, &c_addr.sin_addr.s_addr, client_ip, sizeof(client_ip));

        std::cout << "client ip: " << client_ip << " port: " << client_port << std::endl;
        return true;
    }
    void run() {
        const size_t buffer_size = 1024;
        char buf[buffer_size];
        if(!acceptConn()) {
            std::cout << "connect error" << std::endl;
            return;
        }
        while(true) {
            int num = recv(client_fd, buf, sizeof(buf), 0);
            if(num == -1) {
                std::cerr << "read error" << std::endl;
                return;
            } else if(num > 0) {
                std::cout << "receive client data: " << buf << std::endl;
                std::string msg = "receive success";
                send(client_fd, msg.c_str(), msg.size(), 0);
            } else if(num == 0) {
                std::cout << "client closed" << std::endl;
                break;
            }
            sleep(2);
        }
        std::cout << "server stop" << std::endl;
    }
};
class Client {
  private:
    int socket_fd;
    sockaddr_in server_addr;
    bool ok;
    char buf[1024];
  public:
    Client() = delete;
    Client(std::string ip, int port) {
        socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(socket_fd == -1) {
            perror("socket");
            ok = false;
            return;
        }
        std::memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr.s_addr);
        server_addr.sin_port = htons(port);

        while(connect(socket_fd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
            std::cout << "connect failed, retry..." << std::endl;
            sleep(1);
        }
        std::cout << "connect succeed" << std::endl;
        ok = true;
    }
    ~Client() {
        close(socket_fd);
    }
    int send(const std::string& data) {
        if(!ok) {
            return -1;
        }
        int send_size = ::send(socket_fd, data.c_str(), data.size(), 0);
        if(send_size == -1) {
            perror("send");
            return -1;
        }
        int num = recv(socket_fd, buf, sizeof(buf), 0);
        if(num == -1) {
            perror("recv");
            return -1;
        } else if(num > 0) {
            std::cout << "receive message from server: " << buf << " size: " << num << std::endl;
        } else if(num == 0) {
            std::cout << "server closed" << std::endl;
            return 1;
        }
        return 0;
    }
};
class ResourceManager {
  public:
    static std::shared_ptr<Service> getSimpleTcpService(int port) {
        return std::make_shared<Service>(port);
    }
    static std::shared_ptr<Client> getSimpleTcpClient(std::string ip, int port) {
        return std::make_shared<Client>(ip, port);
    }
};

} // tcp