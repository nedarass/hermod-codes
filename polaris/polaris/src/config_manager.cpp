#include "../include/config_manager.h"
#include <fstream>
#include <iostream>

// Dosya varlık kontrolü
bool ConfigManager::fileExists() const {
    std::ifstream file(configFilename);
    return file.good();
}

ConfigManager::ConfigManager(const std::string& filename)
    : configFilename(filename)
{
    // 1. Önce Varsayılan (Default) Değerleri Yükle
    // Dosya okuma başarısız olursa sistem bu değerlerle çalışır.
    name = "Hermod_Polaris_V1";
    ipAddress = "192.168.1.100";
    port = 12345;
    
    broadcastingAddress = "255.255.255.255";
    broadcastingPort = 12321;

    serialPortName = "/dev/ttyACM0"; // Linux'ta genelde budur
    baudRate = 115200;

    // 2. Dosya varsa üzerine yaz (Load), yoksa oluştur (Save)
    if (fileExists()) {
        if (!loadConfig()) {
            std::cerr << "UYARI: Config dosyasi bozuk, varsayilanlar kullaniliyor." << std::endl;
        } else {
            std::cout << "AYARLAR: Config dosyasi yuklendi." << std::endl;
        }
    } else {
        std::cout << "BILGI: Config dosyasi bulunamadi, varsayilan ayarlarla olusturuluyor..." << std::endl;
        saveConfig();
    }
}

bool ConfigManager::loadConfig() {
    std::ifstream file(configFilename);
    if (!file.is_open()) return false;

    // Satır satır okuma (Sıralama Önemli!)
    // Dosya yapısı: Name \n IP \n Port \n BroadcastIP \n BroadcastPort \n SerialPort \n BaudRate
    
    if (!std::getline(file, name)) return false;
    if (!std::getline(file, ipAddress)) return false;
    
    std::string temp;
    if (!std::getline(file, temp)) return false; port = std::stoi(temp);
    
    if (!std::getline(file, broadcastingAddress)) return false;
    if (!std::getline(file, temp)) return false; broadcastingPort = std::stoi(temp);

    if (!std::getline(file, serialPortName)) return false;
    if (!std::getline(file, temp)) return false; baudRate = std::stoi(temp);

    file.close();
    return true;
}

bool ConfigManager::saveConfig() const {
    std::ofstream outfile(configFilename);
    if (!outfile.is_open()) return false;

    outfile << name << std::endl;
    outfile << ipAddress << std::endl;
    outfile << port << std::endl;
    outfile << broadcastingAddress << std::endl;
    outfile << broadcastingPort << std::endl;
    outfile << serialPortName << std::endl;
    outfile << baudRate << std::endl;

    outfile.close();
    return true;
}

// --- GETTERS ---
std::string ConfigManager::getName() const { return name; }
std::string ConfigManager::getIPAddress() const { return ipAddress; }
int ConfigManager::getPort() const { return port; }
std::string ConfigManager::getBroadcastingIPAddress() const { return broadcastingAddress; }
uint16_t ConfigManager::getBroadcastingPort() const { return broadcastingPort; }
std::string ConfigManager::getSerialPortName() const { return serialPortName; }
int ConfigManager::getBaudRate() const { return baudRate; }

// --- SETTERS ---
void ConfigManager::setName(const std::string& val) { name = val; saveConfig(); }
void ConfigManager::setIPAddress(const std::string& val) { ipAddress = val; saveConfig(); }
void ConfigManager::setPort(const int val) { port = val; saveConfig(); }
void ConfigManager::setBroadcastingIPAddress(const std::string& val) { broadcastingAddress = val; saveConfig(); }
void ConfigManager::setBroadcastingPort(const int val) { broadcastingPort = val; saveConfig(); }
void ConfigManager::setSerialPortName(const std::string& val) { serialPortName = val; saveConfig(); }
void ConfigManager::setBaudRate(const int val) { baudRate = val; saveConfig(); }
