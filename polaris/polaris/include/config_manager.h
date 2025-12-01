#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <cstdint>

class ConfigManager {
public:
    // Yapıcı fonksiyon: Dosya adını alır
    ConfigManager(const std::string& filename = "config.hcf");

    // Dosyadan oku / Dosyaya yaz
    bool loadConfig();
    bool saveConfig() const;

    // --- GETTER (Okuma) Fonksiyonları ---
    std::string getName() const;
    std::string getIPAddress() const;
    int getPort() const;

    std::string getBroadcastingIPAddress() const;
    uint16_t getBroadcastingPort() const;

    // Yeni Eklenenler (Serial)
    std::string getSerialPortName() const;
    int getBaudRate() const;

    // --- SETTER (Yazma) Fonksiyonları ---
    void setName(const std::string& name);
    void setIPAddress(const std::string& ip);
    void setPort(const int port);
    
    void setBroadcastingIPAddress(const std::string& ip);
    void setBroadcastingPort(const int port);

    void setSerialPortName(const std::string& portName);
    void setBaudRate(const int baud);

private:
    std::string configFilename;

    // Config Değişkenleri
    std::string name;
    std::string ipAddress;
    int port;

    std::string broadcastingAddress;
    uint16_t broadcastingPort;

    std::string serialPortName; // Örn: /dev/ttyACM0
    int baudRate;               // Örn: 115200

    // Dosya var mı kontrolü
    bool fileExists() const;
};

#endif
