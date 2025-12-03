#include "../include/serial_manager.h"
#include <iostream>
#include <cstring>   // memcpy
#include <algorithm> // std::find
#include <vector>
#include <iomanip>   // Debug için hex formatı

// Linux Serial Port Kütüphaneleri
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

// --- YARDIMCI FONKSİYON: CRC HESAPLAMA ---
// STM32 tarafındaki (communication.c) algoritmanın aynısıdır.
static uint16_t CalculateCRC(const uint8_t *data, int len) {
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) 
                crc = (crc << 1) ^ 0x1021;
            else 
                crc <<= 1;
        }
    }
    return crc;
}

// --- CONSTRUCTOR ---
SerialManager::SerialManager(const std::string& portName, int baudRate)
    : portName(portName), baudRate(baudRate), serialFd(-1) {
}

// --- DESTRUCTOR ---
SerialManager::~SerialManager() {
    closeSerial();
}

// --- PORT AÇMA VE AYARLAMA ---
bool SerialManager::openSerial() {
    // O_RDWR: Oku/Yaz
    // O_NOCTTY: Terminal olarak atama
    // O_NDELAY: Non-blocking (Veri yoksa bekleme, hemen dön)
    serialFd = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    
    if (serialFd == -1) {
        std::perror("SerialManager: Port acilamadi");
        return false;
    }

    // Mevcut ayarları al
    struct termios options;
    tcgetattr(serialFd, &options);

    // Baud Rate Ayarla (Genelde B115200 kullanılır)
    speed_t baud;
    switch (baudRate) {
        case 9600: baud = B9600; break;
        case 115200: baud = B115200; break;
        case 230400: baud = B230400; break;
        default: baud = B115200; break;
    }
    cfsetispeed(&options, baud);
    cfsetospeed(&options, baud);

    // 8N1 Ayarları (8 Data bits, No Parity, 1 Stop bit)
    options.c_cflag &= ~PARENB; // Parity yok
    options.c_cflag &= ~CSTOPB; // 1 Stop bit
    options.c_cflag &= ~CSIZE;  // Maskeyi temizle
    options.c_cflag |= CS8;     // 8 Data bits

    // Raw Mode (Binary veri için şart - yoksa Linux karakterleri değiştirir)
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Canonical mode kapa
    options.c_oflag &= ~OPOST; // Output processing kapa
    
    // Ayarları uygula
    tcsetattr(serialFd, TCSANOW, &options);
    
    // Non-blocking okuma ayarını kesinleştir
    fcntl(serialFd, F_SETFL, O_NONBLOCK);

    return true;
}

// --- PORT KAPATMA ---
void SerialManager::closeSerial() {
    if (serialFd != -1) {
        close(serialFd);
        serialFd = -1;
    }
}

// --- VERİ OKUMA VE AYRIŞTIRMA (PARSER) ---
// Bu fonksiyon döngü içinde sürekli çağrılır.
std::string SerialManager::readAndParse() {
    if (serialFd == -1) return "";

    // 1. Porttan gelen veriyi oku
    uint8_t tempBuf[256];
    int bytesRead = read(serialFd, tempBuf, sizeof(tempBuf));

    if (bytesRead > 0) {
        // Okunan veriyi ana tampona (Vector) ekle
        rxBuffer.insert(rxBuffer.end(), tempBuf, tempBuf + bytesRead);
    }

    // 2. Tamponda anlamlı paket var mı diye bak
    // Protokol: [AA] [ID] [TYPE] [LEN_L] [LEN_H] [PAYLOAD...] [CRC_L] [CRC_H] [55]
    // En küçük paket (Payloadsız): 1+1+1+2+0+2+1 = 8 Byte
    
    while (rxBuffer.size() >= 8) { 
        
        // A. Başlangıç Baytını (0xAA) Bul
        auto it = std::find(rxBuffer.begin(), rxBuffer.end(), (uint8_t)PKT_START);
        
        if (it == rxBuffer.end()) {
            rxBuffer.clear(); // Hiç başlangıç yoksa hepsi çöptür
            break;
        }
        
        // Eğer başlangıç baytı en başta değilse, öncesindeki çöpleri sil
        if (it != rxBuffer.begin()) {
            rxBuffer.erase(rxBuffer.begin(), it);
        }

        // B. Uzunluk Bilgisini Oku
        // Header boyutu (AA + ID + TYPE + LEN_L + LEN_H) = 5 byte
        if (rxBuffer.size() < 5) break; // Header henüz tamamlanmadı, bekle

        uint8_t id = rxBuffer[1];
        uint8_t type = rxBuffer[2];
        uint16_t payloadLen = rxBuffer[3] | (rxBuffer[4] << 8);

        // Toplam Paket Boyutu = Header(5) + Payload + Footer(CRC:2 + END:1)
        size_t totalPacketSize = 5 + payloadLen + 3;

        // C. Paketin tamamı geldi mi?
        if (rxBuffer.size() < totalPacketSize) {
            // Paket yarım kalmış, bir sonraki read döngüsünü bekle
            break; 
        }

        // D. Bitiş Baytı (0x55) Kontrolü
        if (rxBuffer[totalPacketSize - 1] != PKT_END) {
            // Paket yapısı bozuk (Kayma var), ilk byte'ı sil ve tekrar dene
            rxBuffer.erase(rxBuffer.begin());
            continue;
        }else if (type == 0xFF) { // TYPE_BIN (binary data)
            // Hata bayrakları için
            uint32_t val;
            if (payloadLen >= 4) {
                memcpy(&val, payloadPtr, 4);
                result = "BIN:" + std::to_string(id) + ":" + std::to_string(val);
            }
        }

        // E. CRC (Güvenlik) Kontrolü
        // Paketin sonundaki CRC'yi al
        uint16_t receivedCRC = rxBuffer[totalPacketSize - 3] | (rxBuffer[totalPacketSize - 2] << 8);
        
        // Bizim hesapladığımız CRC (Sadece payload üzerinden, STM32'ye uyumlu)
        const uint8_t* payloadPtr = &rxBuffer[5];
        uint16_t calculatedCRC = CalculateCRC(payloadPtr, payloadLen);

        if (receivedCRC == calculatedCRC) {
            // --- PAKET DOĞRU VE GÜVENLİ ---
            std::string result = "";
            
            // Veri tipine göre dönüştür ve String yap (Örn: "FLOAT:1:25.5")
            // Bu format Bifrost (Arayüz) tarafından kolayca parse edilir.
            
            if (type == 0x07) { // TYPE_F32 (Float)
                float val;
                std::memcpy(&val, payloadPtr, 4);
                result = "FLOAT:" + std::to_string(id) + ":" + std::to_string(val);
            } 
            else if (type == 0x04) { // TYPE_I16 (Int16)
                int16_t val;
                std::memcpy(&val, payloadPtr, 2);
                result = "INT:" + std::to_string(id) + ":" + std::to_string(val);
            } 
            else if (type == 0x01) { // TYPE_U8 (UInt8)
                uint8_t val = payloadPtr[0];
                result = "UINT:" + std::to_string(id) + ":" + std::to_string(val);
            }
            
            // İşlenen paketi tampondan sil
            rxBuffer.erase(rxBuffer.begin(), rxBuffer.begin() + totalPacketSize);
            
            return result; // Bulunan paketi döndür
        } else {
            // CRC Hatası: Paket bozuk gelmiş
            std::cerr << "CRC ERROR! Paket atiliyor." << std::endl;
            rxBuffer.erase(rxBuffer.begin()); // Hatalı paketin başını sil, tekrar dene
        }
    }
    
    return ""; // İşlenecek tam paket yok
}

// --- KOMUT GÖNDERME (Bifrost -> STM32) ---
bool SerialManager::sendCommand(uint8_t cmdId, uint8_t type, const void* payload, int len) {
    if (serialFd == -1) return false;

    std::vector<uint8_t> packet;
    
    // 1. Header
    packet.push_back(PKT_START);
    packet.push_back(cmdId);
    packet.push_back(type);
    
    // 2. Uzunluk (Little Endian)
    packet.push_back(len & 0xFF);
    packet.push_back((len >> 8) & 0xFF);

    // 3. Payload
    const uint8_t* pData = (const uint8_t*)payload;
    for (int i = 0; i < len; i++) {
        packet.push_back(pData[i]);
    }

    // 4. CRC (Payload üzerinden)
    uint16_t crc = CalculateCRC(pData, len);
    packet.push_back(crc & 0xFF);
    packet.push_back((crc >> 8) & 0xFF);

    // 5. Footer
    packet.push_back(PKT_END);

    // 6. Yaz
    int written = write(serialFd, packet.data(), packet.size());
    return (written == (int)packet.size());
}

// Yardımcı fonksiyon: Baudrate çevirici (Header'da tanımlıydı)
speed_t SerialManager::getBaudRateConst(int baud) {
    // Bu fonksiyon openSerial içinde zaten switch-case ile halledildi, 
    // ama header uyumluluğu için boş bırakabilir veya tekrar kullanabilirsin.
    return B115200; 
}
