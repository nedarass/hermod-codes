#include "include/config_manager.h"
#include "include/udp_broadcasting.h"
#include "include/tcp_server.h"
#include "include/serial_manager.h"
#include "include/communication_ids.h" 

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <cstring>

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
       if (!telemetry.empty()) {
            // Artık telemetry binary formatında değil, parse edilmiş string
            // Örnek: "VEL:12.5", "TEMP:25.3", "BRAKE:1"
            tcpServer.sendData(telemetry + "\n");
        }


        // --- C. YÖN: BIFROST -> STM32 (Komutlar) ---
        if (tcpServer.hasData()) {
            std::string binaryData = tcpServer.receiveData();
            
            // Burada binaryData Bifrost'tan gelen binary frame
            // Format: [AA][55][ID][LEN][PAYLOAD...][CRC]
            
            if (binaryData.length() >= 6) {
                const uint8_t* data = reinterpret_cast<const uint8_t*>(binaryData.c_str());
                 // Header kontrolü
                if (data[0] == PKT_START && data[1] == PKT_END) {
                    uint8_t cmdId = data[2];
                    uint8_t payloadLen = data[3];
                    if (binaryData.length() >= (4 + payloadLen + 1)) { // +1 for CRC
                        const uint8_t* payload = &data[4];
                        
                        // CRC kontrolü (basit XOR)
                        uint8_t crc = 0;
                        for (int i = 0; i < 4 + payloadLen; i++) {
                            crc ^= data[i];
                        }
                        
                        uint8_t receivedCrc = data[4 + payloadLen];
                        
                        if (crc == receivedCrc) {
                            // Komutu STM32'ye ilet
                            switch (cmdId) {
                                case CMD_BRAKE_ACTUATE: {
                                    if (payloadLen >= 1) {
                                        uint8_t brakeForce = payload[0];
                                        pulseSerial.sendCommand(CMD_BRAKE_ACTUATE, TYPE_U8, &brakeForce, 1);
                                        std::cout << "[POLARIS] Fren: %" << (int)brakeForce << " -> STM32" << std::endl;
                                    }
                                    break;
                                }
                                case CMD_SET_TARGET_SPEED: {
                                    if (payloadLen >= 4) {
                                        float speed;
                                        memcpy(&speed, payload, 4);
                                        pulseSerial.sendCommand(CMD_SET_TARGET_SPEED, TYPE_F32, &speed, 4);
                                        std::cout << "[POLARIS] Hiz: " << speed << " m/s -> STM32" << std::endl;
                                    }
                                    break;
                                }
                                case CMD_POWER_CUT_OFF: {
                                    pulseSerial.sendCommand(CMD_POWER_CUT_OFF, 0x00, nullptr, 0);
                                    std::cout << "[POLARIS] ACIL GUC KESME -> STM32" << std::endl;
                                    break;
                                }
                                case CMD_PING_REQUEST: {
                                    // Ping'e cevap ver
                                    uint16_t pingResponse = 1; // 1ms latency (simulation)
                                    pulseSerial.sendCommand(ID_PING_RESPONSE, TYPE_I16, &pingResponse, 2);
                                    std::cout << "[POLARIS] Ping Request -> STM32" << std::endl;
                                    break;
                                }
                                default: {
                                    std::cout << "[POLARIS] Bilinmeyen komut ID: 0x" << std::hex << (int)cmdId << std::endl;
                                    break;
                                }
                            }
                        } else {
                            std::cerr << "[POLARIS] CRC hatasi!" << std::endl;
                        }
                    }
                } else if (binaryData == "PING") {
                    // String PING (keep-alive için)
                    tcpServer.sendData("PONG");
                }
            }
        }
    }
    

        // --- D. Periyodik Ping ---
       if (tcpServer.shouldSendPing()) {
            tcpServer.sendPing();
        }

        // CPU Tasarrufu
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return 0;
}
