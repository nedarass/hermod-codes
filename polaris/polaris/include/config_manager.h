#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>

class ConfigManager {
public:
    //polaris config file
    ConfigManager(const std::string& filename = "config.hcf");

    // config verileri dosyadan yukler
    bool loadConfig();

    // bellek -> dosya config veriler
    bool saveConfig() const;

    // bellek erisimciler
    std::string getName() const;
    std::string getIPAddress() const;
    int getPort() const;

    std::string getBroadcastingIPAddress() const;
    u_int16_t getBroadcastingPort() const;

    // degistirme
    void setName(const std::string& name);
    void setIPAddress(const std::string& ip);
    void setPort(const int port);
    
    void setBroadcastingIPAddress(const std::string& ip);
    void setBroadcastingPort(const int port);

private:
    std::string configFilename;
    std::string name;
    std::string ipAddress;
    u_int16_t port;

    std::string broadcastingAddress;
    u_int16_t broadcastingPort;

    // dosya var mi yok mu
    bool fileExists() const;
};

#endif
