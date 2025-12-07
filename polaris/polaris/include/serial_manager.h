#ifndef SERIAL_MANAGER_H
#define SERIAL_MANAGER_H

#include <string>
#include <vector>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdint>
#include "../include/communication_ids.h" // Ortak ID Dosyası

class SerialManager {
public:
    SerialManager(const std::string& portName, int baudRate);
    ~SerialManager();

    bool openSerial();
    void closeSerial();

    // Pulse'dan gelen [AA][55]... paketini yakalayıp döndürür.
    // Eğer paket yoksa boş vector döner.
    std::vector<uint8_t> readRawPacket();

    // Bifrost'tan gelen komutu Pulse'a iletir
    bool sendCommand(uint8_t cmdId, uint8_t type, const std::vector<uint8_t>& payload);

private:
    std::string portName;
    int baudRate;
    int serialFd; // Dosya tanımlayıcısı (Linux portu)
    
    std::vector<uint8_t> rxBuffer; // Gelen veriyi biriktirme havuzu

    // Yardımcı Fonksiyonlar
    speed_t getBaudRateConst(int baud);
    uint8_t calculateCRC(const std::vector<uint8_t>& data);
};

#endif
