#include "include/config_manager.h"
#include "include/udp_broadcasting.h"
#include "include/tcp_server.h"
#include "include/serial_manager.h"
#include "include/communication_ids.h" 

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

int main()
{
    std::cout << "--- POLARIS (Vehicle Bridge) BASLATILIYOR ---" << std::endl;

    // 1. AYARLARI YÜKLE
    // "config.hcf" dosyasını okur. Yoksa default ayarlarla oluşturur.
    ConfigManager config("config.hcf");
    
    std::cout << "Konfigurasyon: " << config.getName() << std::endl;
    std::cout << "Seri Port: " << config.getSerialPortName() << " @ " << config.getBaudRate() << std::endl;

    // 2. MODÜLLERİ OLUŞTUR (Ayarları Config'den alarak)
    
    // UDP Yayıncı (Bifrost'un bizi bulması için)
    UDPBroadcasting udpBroadcaster(
        config.getBroadcastingIPAddress(), 
        config.getBroadcastingPort()
    );

    // TCP Sunucu (Veri akışı için)
    TCPServer tcpServer(config.getPort());

    // Seri Port Yöneticisi (STM32 ile konuşmak için)
    SerialManager pulseSerial(
        config.getSerialPortName(), 
        config.getBaudRate()
    );

    // 3. MODÜLLERİ BAŞLAT
    
    // --- UDP Başlat ---
    try {
        udpBroadcaster.setUpSocket();
        // Yayın Mesajı: "BASLIK \n ISIM \n IP:PORT"
        udpBroadcaster.broadcastMessage = "hermod-polaris-broadcast\n" + 
                                          config.getName() + "\n" + 
                                          config.getIPAddress() + ":" + 
                                          std::to_string(config.getPort());
        
        udpBroadcaster.startRecurrentBroadcastingOnThread();
        std::cout << "[UDP] Yayin basladi." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[UDP HATA] " << e.what() << std::endl;
    }

    // --- Seri Port Başlat ---
    if (pulseSerial.openSerial()) {
        std::cout << "[SERIAL] STM32 Baglandi." << std::endl;
    } else {
        std::cerr << "[SERIAL HATA] STM32 Bulunamadi! Baglantilari kontrol edin." << std::endl;
        // Kritik değil, belki simülasyon yapıyoruzdur. Devam et.
    }

    // --- TCP Sunucu Başlat ---
    if (!tcpServer.start()) {
        std::cerr << "[TCP HATA] Sunucu portu acilamadi!" << std::endl;
        return 1; // TCP olmadan çalışamayız, çık.
    }

    // 4. İSTEMCİ BEKLEME (Blocking)
    std::cout << "[TCP] Bifrost bekleniyor..." << std::endl;
    if (tcpServer.acceptClient()) {
        std::cout << "[TCP] Bifrost Baglandi!" << std::endl;
        udpBroadcaster.stopRecurrentBroadcastingOnThread(); // Artık yayına gerek yok
    }

    // 5. ANA DÖNGÜ (Data Bridge)
    auto lastPingTime = std::chrono::steady_clock::now();

    while (true) {
        
        // --- A. İstemci Kontrolü ---
        // Eğer bağlantı koptuysa tekrar bekle (Basit Reconnect Mantığı)
        if (!tcpServer.isConnected()) {
            std::cout << "[TCP] Baglanti koptu, tekrar bekleniyor..." << std::endl;
            udpBroadcaster.startRecurrentBroadcastingOnThread(); // Yayını tekrar aç
            if (tcpServer.acceptClient()) {
                std::cout << "[TCP] Bifrost Tekrar Baglandi!" << std::endl;
                udpBroadcaster.stopRecurrentBroadcastingOnThread();
            }
        }

        // --- B. YÖN: STM32 -> BIFROST (Telemetri) ---
        // Seri porttan paketleri oku ve çöz
        std::string telemetry = pulseSerial.readAndParse();
        while (!telemetry.empty()) {
            // Gelen veri: "FLOAT:1:25.5" formatında.
            // Bunu TCP üzerinden Arayüze gönder.
            tcpServer.sendData(telemetry + "\n");
            
            // Tamponda başka paket var mı?
            telemetry = pulseSerial.readAndParse();
        }

        // --- C. YÖN: BIFROST -> STM32 (Komutlar) ---
        if (tcpServer.hasData()) {
        std::string command = tcpServer.receiveData();
        std::cout << "[BIFROST] Komut: " << command << std::endl;
        
        // Basit parsing (Bifrost'tan gelen format: "BRAKE 50")
        if (command.find("BRAKE ") == 0) {  // "BRAKE " ile başlıyorsa
            std::string valueStr = command.substr(6); // "50"
            int brakeValue = std::stoi(valueStr);
            if (brakeValue > 100) brakeValue = 100;
            if (brakeValue < 0) brakeValue = 0;
            
            uint8_t payload = static_cast<uint8_t>(brakeValue);
            pulseSerial.sendCommand(0xA1, 0x01, &payload, 1); // CMD_BRAKE_ACTUATE
            
            std::cout << "[POLARIS] Fren: %" << brakeValue << " -> STM32" << std::endl;
        }
        else if (command.find("SPEED ") == 0) {
            float speedValue = std::stof(command.substr(6));
            pulseSerial.sendCommand(0xA2, 0x07, &speedValue, 4); // CMD_SET_TARGET_SPEED
            
            std::cout << "[POLARIS] Hiz: " << speedValue << " m/s -> STM32" << std::endl;
        }
        else if (command == "POWER_CUT") {
            pulseSerial.sendCommand(0xA3, 0x00, nullptr, 0); // CMD_POWER_CUT_OFF
            std::cout << "[POLARIS] ACIL GUC KESME -> STM32" << std::endl;
        }
        else if (command == "PING") {
            tcpServer.sendData("PONG"); // Ping'e cevap ver
        }
    }
    

        // --- D. Periyodik Ping ---
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastPingTime).count() >= 5) {
            tcpServer.sendPing();
            lastPingTime = now;
        }

        // CPU Tasarrufu
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return 0;
}
