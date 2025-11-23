#ifndef SERIAL_MANAGER_H
#define SERIAL_MANAGER_H

#include <string>
#include <vector>
#include <termios.h> // Linux Serial Port
#include <fcntl.h>
#include <unistd.h>
#include <cstdint>

// Pulse tarafındaki communication_ids.h ile uyumlu tanımlar
#define PKT_START  0xAA
#define PKT_END    0x55

// Payload Tipleri
enum PayloadType {
    TYPE_U8  = 0x01,
    TYPE_I16 = 0x04,
    TYPE_F32 = 0x07,
    TYPE_STR = 0x10
};

class SerialManager {
public:
    /**
     * @brief Kurucu fonksiyon
     * @param portName: Örn "/dev/ttyACM0"
     * @param baudRate: Örn 115200
     */
    SerialManager(const std::string& portName, int baudRate);
    ~SerialManager();

    // Portu açar, ayarları (Baudrate, Parity vs) yapar
    bool openSerial();
    
    // Portu kapatır
    void closeSerial();

    /**
     * @brief Porttan veri okur ve anlamlı paketleri ayıklar (Non-blocking)
     * @return Eğer tam bir paket varsa JSON/String formatında döner (Örn: "VEL:120.5"), yoksa boş döner.
     */
    std::string readAndParse();

    /**
     * @brief Pulse'a komut gönderir (Bifrost'tan gelen emirler)
     * @param cmdId: Komut ID (Örn: 0xA1)
     * @param type: Veri Tipi
     * @param payload: Veri içeriği
     */
    bool sendCommand(uint8_t cmdId, uint8_t type, const void* payload, int len);

private:
    std::string portName;
    int baudRate;
    int serialFd; // File Descriptor (Linux'ta port bir dosyadır)
    
    // Gelen verilerin biriktiği tampon
    std::vector<uint8_t> rxBuffer;

    // Yardımcı fonksiyon: Linux baud rate sabitine çevirir
    speed_t getBaudRateConst(int baud);
};

#endif // SERIAL_MANAGER_H
