#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <cstdint>
#include <vector>

class ConfigManager {
public:
    // Yapıcı fonksiyon
    ConfigManager(const std::string& filename = "config.hcf");

    // Dosya İşlemleri
    bool loadConfig();
    bool saveConfig() const;

    // --- GETTER (Okuma) ---
    std::string getName() const;
    std::string getIPAddress() const;
    int getPort() const;

    std::string getBroadcastingIPAddress() const;
    uint16_t getBroadcastingPort() const;

    std::string getSerialPortName() const;
    int getBaudRate() const;

    // --- SETTER (Yazma) - Artık Validasyonlu ---
    void setName(const std::string& name);
    bool setIPAddress(const std::string& ip);
    bool setPort(int port);
    
    bool setBroadcastingIPAddress(const std::string& ip);
    bool setBroadcastingPort(uint16_t port);

    void setSerialPortName(const std::string& portName);
    bool setBaudRate(int baud);

private:
    std::string configFilename;

    // Config Değişkenleri
    std::string name;
    std::string ipAddress;
    int port;

    std::string broadcastingAddress;
    uint16_t broadcastingPort;

    std::string serialPortName;
    int baudRate;

    // Yardımcı Fonksiyonlar
    bool fileExists() const;
    
    // Doğrulama (Validation) Fonksiyonları
    bool isValidIP(const std::string& ip) const;
    bool isValidPort(int p) const;
    bool isValidBaudRate(int baud) const;
};

#endif // CONFIG_MANAGER_H
