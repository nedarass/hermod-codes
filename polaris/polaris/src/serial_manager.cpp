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

// --- YARDIMCI FONKSİYON: CRC HESAPLAMA (Bifrost ile uyumlu) ---
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
        case 19200: baud = B19200; break;
        case 38400: baud = B38400; break;
        case 57600: baud = B57600; break;
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

    // Zaman aşımı ayarları
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 10; // 1 saniye timeout
    
    // Ayarları uygula
    tcsetattr(serialFd, TCSANOW, &options);
    
    // Non-blocking okuma ayarını kesinleştir
    fcntl(serialFd, F_SETFL, O_NONBLOCK);

    std::cout << "Serial port acildi: " << portName << " @ " << baudRate << " baud" << std::endl;

    return true;
}

// --- PORT KAPATMA ---
void SerialManager::closeSerial() {
    if (serialFd != -1) {
        close(serialFd);
        serialFd = -1;
        std::cout << "Serial port kapandi" << std::endl;
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
        
        // A. Başlangıç Baytlarını (0xAA 0x55) Bul
        bool found = false;
        size_t startIdx = 0;
        
        for (size_t i = 0; i <= rxBuffer.size() - 2; i++) {
            if (rxBuffer[i] == PKT_START && rxBuffer[i+1] == PKT_END) {
                found = true;
                startIdx = i;
                break;
            }
        }
        
        if (!found) {
            // Header bulunamadı, tamponu temizle
            rxBuffer.clear();
            break;
        }
        
        // Header'dan önceki verileri sil
        if (startIdx > 0) {
            rxBuffer.erase(rxBuffer.begin(), rxBuffer.begin() + startIdx);
        }

        // B. Paket boyutunu kontrol et
        if (rxBuffer.size() < 4) break; // Header + ID + LEN için minimum 4 byte
        
        quint8 id = rxBuffer[2];
        quint8 len = rxBuffer[3];
        size_t totalPacketSize = 4 + len + 1; // Header(2) + ID(1) + LEN(1) + payload + CRC(1)

        // C. Paketin tamamı geldi mi?
        if (rxBuffer.size() < totalPacketSize) {
            break; // Paket tamamlanmamış
        }

        // D. CRC kontrolü
        QByteArray packetForCRC;
        for (size_t i = 0; i < 4 + len; i++) {
            packetForCRC.append(rxBuffer[i]);
        }
        
        quint8 receivedCrc = rxBuffer[4 + len];
        quint8 calculatedCrc = 0;
        for (char byte : packetForCRC) {
            calculatedCrc ^= static_cast<quint8>(byte);
        }

        if (receivedCrc != calculatedCrc) {
            // CRC hatası, paketi at ve 1 byte kaydır
            rxBuffer.erase(rxBuffer.begin());
            continue;
        }

        // E. Paket geçerli, parse et
        QByteArray payloadData;
        for (size_t i = 4; i < 4 + len; i++) {
            payloadData.append(rxBuffer[i]);
        }

        std::string result = "";
        
        // Telemetri ID'lerine göre parsing (Bifrost ile uyumlu)
        switch (id) {
            case ID_VELOCITY: { // 0x01 - float
                if (len >= 4) {
                    float val;
                    memcpy(&val, payloadData.constData(), 4);
                    result = "VEL:" + std::to_string(val);
                }
                break;
            }
            case ID_ACCELERATION: { // 0x02 - float
                if (len >= 4) {
                    float val;
                    memcpy(&val, payloadData.constData(), 4);
                    result = "ACC:" + std::to_string(val);
                }
                break;
            }
            case ID_POSITION: { // 0x03 - float
                if (len >= 4) {
                    float val;
                    memcpy(&val, payloadData.constData(), 4);
                    result = "POS:" + std::to_string(val);
                }
                break;
            }
            case ID_VOLTAGE: { // 0x04 - uint16 (mV)
                if (len >= 2) {
                    uint16_t raw = static_cast<uint8_t>(payloadData[0]) | 
                                   (static_cast<uint8_t>(payloadData[1]) << 8);
                    float val = raw / 100.0f;
                    result = "VOLT:" + std::to_string(val);
                }
                break;
            }
            case ID_CURRENT: { // 0x05 - int16 (mA)
                if (len >= 2) {
                    int16_t raw = static_cast<uint8_t>(payloadData[0]) | 
                                  (static_cast<uint8_t>(payloadData[1]) << 8);
                    float val = raw / 100.0f;
                    result = "CURR:" + std::to_string(val);
                }
                break;
            }
            case ID_POWER: { // 0x06 - float (W)
                if (len >= 4) {
                    float val;
                    memcpy(&val, payloadData.constData(), 4);
                    result = "PWR:" + std::to_string(val);
                }
                break;
            }
            case ID_TEMPERATURE_NTC1: { // 0x07 - int16 (C * 100)
                if (len >= 2) {
                    int16_t raw = static_cast<uint8_t>(payloadData[0]) | 
                                  (static_cast<uint8_t>(payloadData[1]) << 8);
                    float val = raw / 100.0f;
                    result = "TEMP:" + std::to_string(val);
                }
                break;
            }
            case ID_BRAKE_STATUS: { // 0x08 - uint8
                if (len >= 1) {
                    bool engaged = static_cast<bool>(payloadData[0]);
                    result = "BRAKE:" + std::to_string(engaged);
                }
                break;
            }
            case ID_PING_RESPONSE: { // 0xF1 - uint16 (ms)
                if (len >= 2) {
                    uint16_t pingTime = static_cast<uint8_t>(payloadData[0]) | 
                                       (static_cast<uint8_t>(payloadData[1]) << 8);
                    result = "PING:" + std::to_string(pingTime);
                }
                break;
            }
            case ID_ERROR_FLAG: { // 0xF3 - uint32
                if (len >= 4) {
                    uint32_t flags = static_cast<uint8_t>(payloadData[0]) |
                                    (static_cast<uint8_t>(payloadData[1]) << 8) |
                                    (static_cast<uint8_t>(payloadData[2]) << 16) |
                                    (static_cast<uint8_t>(payloadData[3]) << 24);
                    result = "ERR:" + std::to_string(flags);
                }
                break;
            }
            default: {
                result = "UNK:" + std::to_string(id);
                break;
            }
        }

        // İşlenen paketi tampondan sil
        rxBuffer.erase(rxBuffer.begin(), rxBuffer.begin() + totalPacketSize);
        
        return result;
    }
    
    return ""; // İşlenecek tam paket yok
}

// --- KOMUT GÖNDERME (Bifrost -> STM32) ---
bool SerialManager::sendCommand(uint8_t cmdId, uint8_t type, const void* payload, int len) {
    if (serialFd == -1) return false;

    std::vector<uint8_t> packet;
    
    // Bifrost formatı: [AA][55][ID][LEN][PAYLOAD...][CRC]
    packet.push_back(PKT_START);
    packet.push_back(PKT_END);
    packet.push_back(cmdId);
    packet.push_back(len & 0xFF);

    // 3. Payload
    const uint8_t* pData = (const uint8_t*)payload;
    for (int i = 0; i < len; i++) {
        packet.push_back(pData[i]);
    }

     // CRC hesapla (header + id + len + payload)
    quint8 crc = 0;
    for (size_t i = 0; i < packet.size(); i++) {
        crc ^= packet[i];
    }
    packet.push_back(crc);

    // Serial port'a yaz
    int written = write(serialFd, packet.data(), packet.size());
    
    if (written == (int)packet.size()) {
        std::cout << "Komut gonderildi: ID=0x" << std::hex << (int)cmdId 
                  << ", Len=" << std::dec << len << std::endl;
        return true;
    } else {
        std::cerr << "Komut gonderilemedi!" << std::endl;
        return false;
}

// Yardımcı fonksiyon: Baudrate çevirici (Header'da tanımlıydı)
// Yardımcı fonksiyon
speed_t SerialManager::getBaudRateConst(int baud) {
    switch (baud) {
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        case 230400: return B230400;
        default: return B115200;
    }
}
