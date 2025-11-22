#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <queue>
#include <thread>
#include <chrono>
#include <stdio.h>

#include "../include/tcp_server.h"

TCPServer::TCPServer(int port) : port(port), server_fd(-1), client_fd(-1){}

TCPServer::~TCPServer() {
    closeServer();
}

bool TCPServer::start() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Soket olusturma hatasi" << std::endl;
        return false;
    }
    
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Baglama hatasi" << std::endl;
        return false;
    }
    
    if (listen(server_fd, 1) < 0) {
        std::cerr << "Dinleme hatasi" << std::endl;
        return false;
    }
    
    std::cout << "Sunucu calisiyor (TCP): " << port << std::endl;
    return true;
}

bool TCPServer::acceptClient() {
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        std::cerr << "Client kabul edilmedi" << std::endl;
        return false;
    }
    std::cout << "Client baglandi" << std::endl;

    pingPongLoop();

    return true;
}

std::string TCPServer::receiveData() {
    char buffer[1024] = {0};

    ssize_t bytesReceived = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived == 0) {
        std::cout << "Bağlantı istemci tarafından sonlandırıldı." << std::endl;
        close(client_fd);
        client_fd = -1;
        return "<|closed|>";
    }

    if (bytesReceived == -1) {
        std::cout << "Ağ problemi!" << std::endl;
        close(client_fd);
        client_fd = -1;
        return "<|error|>";
    }

    return std::string(buffer);
}

void TCPServer::sendData(const std::string& data) {
    if (!isConnected()) return;
    const char* c_data = data.c_str();
    
    send(client_fd, c_data, strlen(c_data), 0);
}

void TCPServer::closeServer() {
    if (client_fd != -1) close(client_fd);
    if (server_fd != -1) close(server_fd);
}

//bura duzenlenecek
// BASİT PING GÖNDER
void TCPServer::sendPing() {
    if (!isConnected()) return;
    sendData("PING");
    std::cout << "PING gönderildi -> Pulse" << std::endl;
}
/*void TCPServer::pingPongLoop() {
    bool isError = false;
    
    while (!isError) {
        std::this_thread::sleep_for(std::chrono::milliseconds(pingPongWaitMS));
            
        sendData("PING");
        
        std::string received = receiveData();

        if((received == "<|closed|>") || received == "<|error|>")
        {
            std::cout << received;
            break;    
        }

        std::cout<<received;
    }
}*/

void TCPServer::enqueueMessage(const std::string& message) {
    messageQueue.push(message);
}
