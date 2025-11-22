#include "include/config_manager.h"
#include "include/udp_broadcasting.h"
#include "include/tcp_server.h"
#include <iostream>
#include <thread>
#include <chrono>

int main()
{
    ConfigManager configManager("config.hcf");
    
    UDPBroadcasting udpBroadcaster(
        configManager.getBroadcastingIPAddress(), 
        configManager.getBroadcastingPort()
    );
    udpBroadcaster.setUpSocket();

    udpBroadcaster.broadcastMessage = 
        std::string("hermod-polaris-broadcast") + "\n" + 
        configManager.getName() + "\n" + 
        configManager.getIPAddress() + ":" + 
        std::to_string(configManager.getPort());

    udpBroadcaster.startRecurrentBroadcastingOnThread();

    TCPServer server(configManager.getPort());

    // Sunucuyu başlat
    if (!server.start()) {
        std::cerr << "Sunucu başlatılamadı." << std::endl;
        return 1;
    }

    // Bağlantı kabul et
    std::cout << "Pulse bağlantısı bekleniyor..." << std::endl;
    if (!server.acceptClient()) {
        std::cerr << "Bağlantı kabul edilemedi." << std::endl;
        return 1;
    }

    udpBroadcaster.stopRecurrentBroadcastingOnThread();
    std::cout << "Pulse bağlandı! Veri iletimi başlıyor..." << std::endl;

    // ANA İŞLEM DÖNGÜSÜ
    auto lastPingTime = std::chrono::steady_clock::now();
    int loopCounter = 0;
    
    while (server.isConnected()) {
        loopCounter++;
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastPingTime).count();
        
        // Her 5 saniyede bir PING gönder
        if (elapsed >= 5) {
            server.sendData("PING");
            lastPingTime = now;
            std::cout << "PING gönderildi" << std::endl;
        }
        
        // Pulse'tan gelen verileri oku
        std::string receivedData = server.receiveData();
        
        if (receivedData == "<|closed|>" || receivedData == "<|error|>") {
            std::cout << "Pulse bağlantısı koptu!" << std::endl;
            break;
        }
        
        if (!receivedData.empty()) {
            if (receivedData == "PONG") {
                std::cout << "PONG alındı" << std::endl;
            } else {
                // Pulse'tan gelen telemetri verisi
                std::cout << "PULSE VERISI: " << receivedData << std::endl;
                
                // BURASI: Bu veriyi Bifrost'a ilet
                // Şu an sadece ekrana yazdırıyoruz
                // İleride buraya WebSocket veya HTTP kodları gelecek
            }
        }
        
        // Küçük bekleme
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Polaris programı sonlandı." << std::endl;
    return 0;
}
