#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <queue>
#include <arpa/inet.h>

class TCPServer {
public:
    TCPServer(int port);
    
    ~TCPServer();
    
    bool start();
    
    bool acceptClient();
    
    std::string receiveData();
    
    void sendData(const std::string& data);
    
    void closeServer();
    
    void pingPongLoop();

    void enqueueMessage(const std::string& message);

private:
    int port;
    int server_fd, client_fd;

    int pingPongWaitMS = 100; //ping pong sinyal bekleme süresi.

    std::queue<std::string> messageQueue;
};

#endif
