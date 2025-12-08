#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <chrono>

class TCPServer {
public:
    TCPServer(int port);
    ~TCPServer();
    
    bool start();
    bool acceptClient();

    std::string receiveData();
    std::vector<uint8_t> receiveBinaryData();


    bool sendBinaryData(const std::vector<uint8_t>& data);
    void sendData(const std::string& data);

    void closeServer();
    bool hasData();
    

    void sendPing();
    bool shouldSendPing();  // Zaman kontrolü
    void updateLastPingTime(); // Zamanı güncelle
    
    bool isConnected() const { return client_fd != -1; }

private:
    int port;
    int server_fd, client_fd;
    
    // ✅ Ping-pong state'i TCP Server'da
    std::chrono::steady_clock::time_point lastPingTime;
    const int PING_INTERVAL_MS = 5000; // 5 saniye
};

#endif
