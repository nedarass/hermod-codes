#include "include/config_manager.h"
#include "include/udp_broadcasting.h"
#include "include/tcp_server.h"
#include "include/serial_manager.h" 

#include <iostream>
#include <thread>
#include <chrono>

int main()
{
    //ayarları yükler
    ConfigManager configManager("config.hcf");
    
    // udp yayını (bifrost'un bizi bulması için)
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

    //pulse (STM32) bağlantısı (SERİ PORT)
    // Port adı genelde /dev/ttyACM0 veya /dev/ttyUSB0 olur , ileride bunu config dosyasından çekebiliriz.
    std::cout << "Pulse (STM32) aranıyor..." << std::endl;
    SerialManager pulseSerial("/dev/ttyACM0", 115200);
    
    bool isPulseConnected = pulseSerial.openSerial();
    if (!isPulseConnected) {
        std::cerr << "UYARI: Pulse (STM32) Seri Portuna bağlanılamadı!" << std::endl;
        // Programı kapatmıyoruz, belki sadece arayüz testi yapılıyordur.
    } else {
        std::cout << "BAŞARILI: Pulse (STM32) bağlandı." << std::endl;
    }

    // 4. bifrost (arayüz) bağlantısı (tcp server)
    TCPServer server(configManager.getPort());

    if (!server.start()) {
        std::cerr << "TCP Sunucu başlatılamadı." << std::endl;
        return 1;
    }

    // bağlantı kabul et (bifrost'un bağlanmasını bekle)
    std::cout << "Bifrost (Arayüz) bağlantısı bekleniyor..." << std::endl;
    if (!server.acceptClient()) {
        std::cerr << "Bifrost bağlantısı kabul edilemedi." << std::endl;
        return 1;
    }

    udpBroadcaster.stopRecurrentBroadcastingOnThread();
    std::cout << "Bifrost bağlandı! Sistem tam aktif." << std::endl;

    //ana işlem döngüsü
    auto lastPingTime = std::chrono::steady_clock::now();
    
    while (server.isConnected()) {
        
        // A) yön: pulse (STM32) -> polaris -> bifrpst
        if (isPulseConnected) {
            // pulse'tan veri var mı diye bak (Non-blocking)
            std::string telemetryData = pulseSerial.readAndParse();
            
            if (!telemetryData.empty()) {
                
                // veriyi direkt bifrost'a TCP üzerinden ilet
                server.sendData(telemetryData);
                
                // debug için (Çok hızlı akıyorsa kapatılabilir)
                // std::cout << "Pulse->Bifrost: " << telemetryData << std::endl;
            }
        }

       
        // yön: bifrost -> polaris -> pulse (STM32)
        
        // Bifrost'tan gelen komutları oku (Non-blocking olması tercih edilir)
        // Not: receiveData şu anki haliyle blokluyor olabilir, 
        // TCP sınıfını non-blocking yapmak gerekebilir.
        /* std::string commandData = server.receiveData(); 
        if (!commandData.empty()) {
            // Gelen komutu analiz et ve Pulse'a gönder
            // Örn: if (commandData == "BRAKE") pulseSerial.sendCommand(...);
        }
        */

        // periyodik işlemler (ping)
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastPingTime).count();
        
        // Her 5 saniyede bir Bifrost'a PING at (Bağlantı kontrolü)
        if (elapsed >= 5) {
            server.sendData("PING");
            lastPingTime = now;
        }
        
        // İşlemciyi %100 kullanmamak için minik bekleme
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    // Bağlantı koparsa portu kapat
    pulseSerial.closeSerial();
    std::cout << "Polaris programı sonlandı." << std::endl;
    return 0;
}
























/*#include "include/config_manager.h"
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
         if (server.shouldSendPing()) {
            server.sendPing();
        }
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
*/
