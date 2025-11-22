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
     // Bağlantı kurulunca ilk ping zamanını sıfırla
    lastPingTime = std::chrono::steady_clock::now();

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

// ✅ PING-PONG FONKSİYONLARI
void TCPServer::sendPing() {
    if (!isConnected()) return;
    
    sendData("PING");
    updateLastPingTime(); // Gönderince zamanı güncelle
    std::cout << "PING -> Pulse" << std::endl;
}

bool TCPServer::shouldSendPing() {
    if (!isConnected()) return false;
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - lastPingTime).count();
    
    return elapsed >= PING_INTERVAL_MS;
}

void TCPServer::updateLastPingTime() {
    lastPingTime = std::chrono::steady_clock::now();
}

void TCPServer::closeServer() {
    if (client_fd != -1) close(client_fd);
    if (server_fd != -1) close(server_fd);
}

