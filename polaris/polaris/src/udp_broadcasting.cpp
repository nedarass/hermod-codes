#include "../include/udp_broadcasting.h"
#include <chrono>
#include <thread>
#include <atomic>
#include <netinet/in.h>

UDPBroadcasting::UDPBroadcasting(const std::string& broadcastIP, uint16_t port): broadcastIP(broadcastIP), port(port) {};

UDPBroadcasting::~UDPBroadcasting()
{
    stopRecurrentBroadcastingOnThread();

    if (sock >= 0) {
        close(sock);
    }
};

void UDPBroadcasting::setUpSocket()
{
    //udp soket olusturma
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Soket olusturulamasi");
        throw std::runtime_error("Soket olusturulamadi");
    }

    // broadcast izni
    int broadcastEnable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
        perror("Broadcast secenegi ayarlanamadi");
        close(sock);
        throw std::runtime_error("Broadcast secenegi ayarlanamadi");
    }

    // yayin adresi
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(port);
    broadcastAddr.sin_addr.s_addr = inet_addr(broadcastIP.c_str());
}

bool UDPBroadcasting::broadcast()
{


    ssize_t sentBytes = sendto(sock, broadcastMessage.c_str(), broadcastMessage.size(), 0,
        reinterpret_cast<struct sockaddr*>(&broadcastAddr), sizeof(broadcastAddr));
    if (sentBytes < 0) {
        perror("Mesaj gonderilemedi");
        return false;
    }

    std::cout << "Mesaj yayninlandi: " << broadcastIP << " : " << port << "\n";

    return true;
}

void UDPBroadcasting::recurrentBroadcasting()
{
    while (true) {
        // 3 saniye bekle.
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        if(stopRequested.load())
        {
            break;
        }
        
        if (!broadcast()) {
            std::cerr << "Broadcast islemi basarisiz oldu\n";
        }
        
    }
}

void UDPBroadcasting::startRecurrentBroadcastingOnThread()
{
    stopRequested.store(false);
    broadcastThread = new std::thread(&UDPBroadcasting::recurrentBroadcasting, this);
}

void UDPBroadcasting::stopRecurrentBroadcastingOnThread()
{
    stopRequested.store(true);
    if (broadcastThread && broadcastThread->joinable()) {
        broadcastThread->join();
        delete broadcastThread;
        broadcastThread = nullptr;
    }
}
