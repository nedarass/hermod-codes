#include "include/config_manager.h"
#include "include/udp_broadcasting.h"
#include "include/tcp_server.h"

int main()
{
    ConfigManager* configManager = new ConfigManager("config.hcf");
    UDPBroadcasting* udpBroadcaster = new UDPBroadcasting(configManager->getBroadcastingIPAddress(), configManager->getBroadcastingPort());
    udpBroadcaster->setUpSocket();

    udpBroadcaster->broadcastMessage = std::string("hermod-polaris-broadcast") + "\n" + configManager->getName() + "\n" + configManager->getIPAddress() + ":" + std::to_string(configManager->getPort());

    udpBroadcaster->startRecurrentBroadcastingOnThread();


    TCPServer server(configManager->getPort());

    //sunucuyu baslat
    if (!server.start()) {
        std::cerr << "Sunucu başlatılamadı." << std::endl;
        return 1;
    }

    //  baglanti kabul et
    if (server.acceptClient() < 0) {
        std::cerr << "Bağlantı kabul edilemedi." << std::endl;
        return 1;
    }

    udpBroadcaster->stopRecurrentBroadcastingOnThread();

    // ANA İŞLEM DÖNGÜSÜ - PULSE ↔ BIFROST
        while (true) {
            // Pulse'tan veri al
            std::string pulseData = server.receiveData();
            
            if (pulseData == "<|closed|>" || pulseData == "<|error|>") {
                std::cout << "Polaris: Pulse bağlantısı koptu" << std::endl;
                break;
            }
            
            if (!pulseData.empty()) {
                // 🔄 BURADA: Veriyi işle ve Bifrost'a ilet
                // Örnek: JSON'a çevir, WebSocket'e gönder, vs.
                processAndForwardToBifrost(pulseData);
            }
            
            // Bifrost'tan gelen komutları Pulse'a ilet
            checkAndSendCommandsToPulse(server);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Polaris Hatası: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
