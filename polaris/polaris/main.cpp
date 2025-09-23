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

    int a = 0;
    std::cin>>a;
}
