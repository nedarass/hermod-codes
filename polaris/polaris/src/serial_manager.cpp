#include "../include/serial_manager.h"
#include <iostream>
#include <cstring> // memcpy, memset
#include <algorithm> // erase

SerialManager::SerialManager(const std::string& portName, int baudRate)
    : portName(portName), baudRate(baudRate), serialFd(-1) {
}

SerialManager::~SerialManager() {
    closeSerial();
}

bool SerialManager::openSerial() {
    // Dosyayı aç (Read/Write, No controlling terminal, Non-blocking)
    serialFd = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    
    if (serialFd == -1) {
        std::perror("SerialManager: Port açılamadı");
        return false;
    }

    // Mevcut ayarları al
    struct termios options;
    tcgetattr(serialFd, &options);

    // Baud Rate ayarla
    speed_t baud = getBaudRateConst(baudRate);
    cfsetispeed(&options, baud);
    cfsetospeed(&options, baud);

    // 8N1 Ayarları (8 Data bits, No Parity, 1 Stop bit)
    options.c_cflag &= ~PARENB; // Parity yok
    options.c_cflag &= ~CSTOPB; // 1 Stop bit
    options.c_cflag &= ~CSIZE;  // Maskeyi temizle
    options.c_cflag |= CS8;     // 8 Data bits

    // Raw Mode (Binary veri için şart)
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;

    // Ayarları uygula
    tcsetattr(serialFd, TCSANOW, &options);
    
    // Blocking davranışını düzelt (Read hemen dönsün)
    fcntl(serialFd, F_SETFL, 0);

    std::cout << "SerialManager: Bağlantı kuruldu -> " << portName << std::endl;
    return true;
}

void SerialManager::closeSerial() {
    if (serialFd != -1) {
        close(serialFd);
        serialFd = -1;
    }
}

std::string SerialManager::readAndParse() {
    if (serialFd == -1) return "";

    uint8_t tempBuf[256];
    // Porttan oku
    int bytesRead = read(serialFd, tempBuf, sizeof(tempBuf));

    if (bytesRead > 0) {
        // Okunanları ana tampona ekle
        rxBuffer.insert(rxBuffer.end(), tempBuf, tempBuf + bytesRead);
    }

    // --- PARSER (AYRIŞTIRICI) ---
    // Protokol: [AA] [ID] [TYPE] [PAYLOAD...] [55]
    
    while (rxBuffer.size() >= 4) { // En küçük paket 4 byte (Payloadsız varsayımı)
        
        // 1. Başlangıcı (0xAA) bul
        auto it = std::find(rxBuffer.begin(), rxBuffer.end(), (uint8_t)PKT_START);
        
        if (it == rxBuffer.end()) {
            // Başlangıç yoksa hepsi çöptür, temizle
            rxBuffer.clear();
            break;
        }

        // Çöp veriyi baştan sil
        if (it != rxBuffer.begin()) {
            rxBuffer.erase(rxBuffer.begin(), it);
        }

        // Şimdi rxBuffer[0] kesinlikle 0xAA.
        // Yeterli veri var mı diye bakmak için TYPE'ı (rxBuffer[2]) kontrol etmemiz lazım.
        if (rxBuffer.size() < 3) break; // Henüz TİP byte'ı gelmedi

        uint8_t id = rxBuffer[1];
        uint8_t type = rxBuffer[2];
        int payloadSize = 0;

        // communication_ids.h içindeki tiplere göre boyut belirle
        switch (type) {
            case TYPE_U8:  payloadSize = 1; break;
            case TYPE_I16: payloadSize = 2; break;
            case TYPE_F32: payloadSize = 4; break;
            default: payloadSize = 0; break; // Bilinmeyen tip
        }

        int totalPacketSize = 3 + payloadSize + 1; // Header(3) + Payload + End(1)

        if (rxBuffer.size() < (size_t)totalPacketSize) {
            // Paket henüz tamamlanmadı, bekle
            break;
        }

        // 2. Bitiş (0x55) Kontrolü
        if (rxBuffer[totalPacketSize - 1] != PKT_END) {
            // Paket bozuk, ilk byte'ı sil ve tekrar dene
            rxBuffer.erase(rxBuffer.begin());
            continue;
        }

        // --- GEÇERLİ PAKET YAKALANDI ---
        std::string result = "";
        
        // Veriyi çözümle (Binary -> Value)
        if (type == TYPE_F32) {
            float val;
            // Byte array -> Float (Little Endian varsayımı - STM32 & RPi uyumlu)
            std::memcpy(&val, &rxBuffer[3], 4);
            result = "FLOAT:" + std::to_string(id) + ":" + std::to_string(val);
        } 
        else if (type == TYPE_I16) {
            int16_t val;
            std::memcpy(&val, &rxBuffer[3], 2);
            result = "INT:" + std::to_string(id) + ":" + std::to_string(val);
        }
        else if (type == TYPE_U8) {
            uint8_t val = rxBuffer[3];
            result = "UINT:" + std::to_string(id) + ":" + std::to_string(val);
        }

        // İşlenen paketi tampondan sil
        rxBuffer.erase(rxBuffer.begin(), rxBuffer.begin() + totalPacketSize);

        // Bulunan sonucu döndür (Bifrost'a gönderilmek üzere)
        return result; 
    }

    return ""; // Tam paket yoksa boş dön
}

bool SerialManager::sendCommand(uint8_t cmdId, uint8_t type, const void* payload, int len) {
    if (serialFd == -1) return false;

    std::vector<uint8_t> packet;
    packet.push_back(PKT_START);
    packet.push_back(cmdId);
    packet.push_back(type);

    const uint8_t* pData = (const uint8_t*)payload;
    for (int i = 0; i < len; i++) {
        packet.push_back(pData[i]);
    }

    packet.push_back(PKT_END);

    int written = write(serialFd, packet.data(), packet.size());
    return (written == (int)packet.size());
}

speed_t SerialManager::getBaudRateConst(int baud) {
    switch (baud) {
        case 9600: return B9600;
        case 115200: return B115200;
        case 230400: return B230400;
        default: return B115200;
    }
}
