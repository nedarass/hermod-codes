#ifndef UDP_BROADCASTING_H
#define UDP_BROADCASTING_H

#include <iostream>
#include <string>
#include <cstring>      // memset, strlen
#include <stdexcept>    // std::runtime_error
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>     // close()
#include <thread>
#include <atomic>
#include <netinet/in.h>


class UDPBroadcasting {
public:
    std::string broadcastIP = "255.255.255.255";
    uint16_t port = 12312;

    std::string broadcastMessage;
    
    //broadcast adresi ve port ile construct
    UDPBroadcasting(const std::string& broadcastIP, uint16_t port);
    
    // soket kapa destructor
    ~UDPBroadcasting();

    void setUpSocket();

    // udp broadcasting
    bool broadcast();

    void startRecurrentBroadcastingOnThread();
    void stopRecurrentBroadcastingOnThread();

private:
    int sock;
    struct sockaddr_in broadcastAddr;

    std::thread* broadcastThread;
    void recurrentBroadcasting();
    
    std::atomic<bool> stopRequested;
};

#endif
