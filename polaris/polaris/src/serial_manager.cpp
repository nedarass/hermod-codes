#include "../include/serial_manager.h"
#include <iostream>
#include <cstring>
#include <vector>
#include <iomanip>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

// --- CONSTRUCTOR ---
SerialManager::SerialManager(const std::string& portName, int baudRate)
    : portName(portName), baudRate(baudRate), serialFd(-1) {
}

SerialManager::~SerialManager() {
    closeSerial();
}

// --- PORT AÇMA (LINUX AYARLARI) ---
bool SerialManager::openSerial() {
    // Portu aç: Read/Write, Controlling TTY yok, Non-blocking değil (başlangıçta)
    serialFd = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    
    if (serialFd == -1) {
        std::perror("[SerialManager] Port acilamadi");
        return false;
    }

    // Mevcut ayarları al
    struct termios options;
    tcgetattr(serialFd, &options);

    // Baud Rate Ayarla
    speed_t baud = getBaudRateConst(baudRate);
    cfsetispeed(&options, baud);
    cfsetospeed(&options, baud);

    // 8N1 Ayarları (8 Data bits, No Parity, 1 Stop bit)
    options.c_cflag &= ~PARENB; // Parity yok
    options.c_cflag &= ~CSTOPB; // 1 Stop bit
    options.c_cflag &= ~CSIZE;  
    options.c_cflag |= CS8;     // 8 Data bits
    
    // Modem kontrollerini devre dışı bırak (Local connection, Enable receiver)
    options.c_cflag |= (CLOCAL | CREAD);

    // Raw Mode (Binary veri için şart! Yoksa Linux 0x0A'yı 0x0D'ye çevirir vs.)
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;
    options.c_iflag &= ~(IXON | IXOFF | IXANY); // Yazılımsal akış denetimini kapa
    
    // Ayarları uygula
    tcsetattr(serialFd, TCSANOW, &options);
    
    // Portu Non-blocking moda al (Okuma yaparken program donmasın)
    fcntl(serialFd, F_SETFL, O_NONBLOCK);

    std::cout << "[SerialManager] Port acildi: " << portName << " @ " << baudRate << std::endl;
    return true;
}

void SerialManager::closeSerial() {
    if (serialFd != -1) {
        close(serialFd);
        serialFd = -1;
        std::cout << "[SerialManager] Port kapatildi." << std::endl;
    }
}

// --- CRC HESAPLAMA (XOR - Bifrost/Pulse ile Aynı) ---
uint8_t SerialManager::calculateCRC(const std::vector<uint8_t>& data) {
    uint8_t crc = 0;
    for (uint8_t byte : data) {
        crc ^= byte;
    }
    return crc;
}

// --- PAKET OKUMA (BINARY PASS-THROUGH) ---
// Bu fonksiyon döngü içinde çağrılır. 
// Tam bir paket ([AA]...[CRC]) yakalarsa döndürür, yoksa boş döner.
std::vector<uint8_t> SerialManager::readRawPacket() {
    if (serialFd == -1) return {};

    // 1. Porttan yeni gelen veriyi oku
    uint8_t tempBuf[256];
    int bytesRead = read(serialFd, tempBuf, sizeof(tempBuf));

    if (bytesRead > 0) {
        // Okunan veriyi ana havuza ekle
        rxBuffer.insert(rxBuffer.end(), tempBuf, tempBuf + bytesRead);
    }

    // 2. Havuzda anlamlı paket var mı diye bak
    // En küçük paket: [AA][55][ID][LEN][CRC] = 5 byte (Payload 0 ise)
    // Standart paket: 4 byte payload olsa 9 byte eder.
    
    while (rxBuffer.size() >= 5) { 
        
        // A. Header Kontrolü (AA 55) - communication_ids.h'den gelir
        if (rxBuffer[0] != PKT_START || rxBuffer[1] != PKT_END) {
             // Header değilse, 1 byte kaydır ve tekrar dene (Senkronizasyon)
             rxBuffer.erase(rxBuffer.begin());
             continue;
        }
        
        // B. Uzunluk Kontrolü
        // [AA][55][ID][LEN] -> Len 4. byte'tadır (indeks 3)
        uint8_t payloadLen = rxBuffer[3];
        size_t totalFrameSize = 4 + payloadLen + 1; // Header(2) + ID(1) + Len(1) + Payload + CRC(1)

        // C. Paketin tamamı gelmiş mi?
        if (rxBuffer.size() < totalFrameSize) {
            // Paket henüz tamamlanmadı, bir sonraki döngüyü bekle
            return {}; 
        }

        // D. Paketi Kopyala
        std::vector<uint8_t> frame(rxBuffer.begin(), rxBuffer.begin() + totalFrameSize);

        // E. CRC Doğrulama
        // CRC son byte hariç hesaplanır
        std::vector<uint8_t> dataToCheck(frame.begin(), frame.end() - 1);
        uint8_t calculated = calculateCRC(dataToCheck);
        uint8_t received = frame.back();

        if (calculated == received) {
            // ✅ GEÇERLİ PAKET
            // Havuzdan bu paketi sil (işlendi)
            rxBuffer.erase(rxBuffer.begin(), rxBuffer.begin() + totalFrameSize);
            
            // Paketi olduğu gibi döndür (Polaris bunu TCP'ye basacak)
            return frame; 
        } else {
            // ❌ CRC HATASI
            std::cerr << "[SerialManager] CRC Hatasi! Beklenen: " << (int)calculated 
                      << " Gelen: " << (int)received << std::endl;
            
            // Header'ı sil ve aramaya devam et (belki ilerde başka header vardır)
            rxBuffer.erase(rxBuffer.begin()); 
            continue;
        }
    }

    return {}; // Tam paket yoksa boş dön
}

// --- KOMUT GÖNDERME (Bifrost -> Pulse) ---
bool SerialManager::sendCommand(uint8_t cmdId, uint8_t type, const std::vector<uint8_t>& payload) {
    if (serialFd == -1) return false;

    std::vector<uint8_t> packet;
    
    // Paket Oluşturma: [AA][55][ID][LEN][PAYLOAD][CRC]
    packet.push_back(PKT_START); 
    packet.push_back(PKT_END);   
    packet.push_back(cmdId);
    // Not: Pulse tarafında Len parametresi payload uzunluğunu ifade eder.
    packet.push_back(static_cast<uint8_t>(payload.size()));

    // Payload Ekle
    packet.insert(packet.end(), payload.begin(), payload.end());

    // CRC Ekle (Tüm paket üzerinden)
    packet.push_back(calculateCRC(packet));

    // Gönder
    int written = write(serialFd, packet.data(), packet.size());
    
    if (written == (int)packet.size()) {
        // Başarılı
        return true;
    } else {
        std::perror("[SerialManager] Yazma hatasi");
        return false;
    }
}

// --- BAUD RATE ÇEVİRİCİ ---
speed_t SerialManager::getBaudRateConst(int baud) {
    switch (baud) {
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        case 230400: return B230400;
        case 460800: return B460800;
        default: return B115200;
    }
}
