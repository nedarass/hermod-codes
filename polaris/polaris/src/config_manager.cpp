
#include "../include/config_manager.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <algorithm> // std::find

// --- YARDIMCI DOĞRULAMA FONKSİYONLARI ---

bool ConfigManager::isValidIP(const std::string& ip) const {
    // Regex ile IPv4 format kontrolü (xxx.xxx.xxx.xxx)
    const std::regex ipPattern(R"(^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$)");
    std::smatch match;
    
    if (std::regex_match(ip, match, ipPattern)) {
        for (int i = 1; i <= 4; ++i) {
            int octet = std::stoi(match[i].str());
            if (octet < 0 || octet > 255) return false;
        }
        return true;
    }
    return false;
}

bool ConfigManager::isValidPort(int p) const {
    return (p > 1024 && p <= 65535); // 0-1024 arası rezerve portlardır
}

bool ConfigManager::isValidBaudRate(int baud) const {
    // Desteklenen yaygın baud rate'ler
    const std::vector<int> validRates = {9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600};
    return std::find(validRates.begin(), validRates.end(), baud) != validRates.end();
}

bool ConfigManager::fileExists() const {
    std::ifstream file(configFilename);
    return file.good();
}

// --- CONSTRUCTOR ---

ConfigManager::ConfigManager(const std::string& filename) : configFilename(filename) {
    // 1. Varsayılan (Güvenli) Değerleri Ata
    name = "Hermod_Polaris_Gateway";
    ipAddress = "192.168.1.100";
    port = 12345;
    
    broadcastingAddress = "255.255.255.255";
    broadcastingPort = 12321;

    // Platforma göre doğru port ismini seç
    #ifdef _WIN32
        serialPortName = "COM3";
    #else
        serialPortName = "/dev/ttyACM0";
    #endif
    
    baudRate = 115200;

    // 2. Dosya varsa yüklemeyi dene, yoksa oluştur
    if (fileExists()) {
        if (loadConfig()) {
            std::cout << "[ConfigManager] Ayarlar yuklendi: " << configFilename << std::endl;
        } else {
            std::cerr << "[ConfigManager] Dosya bozuk veya hatali, varsayilanlar kullaniliyor." << std::endl;
            saveConfig(); // Doğru formatta tekrar oluştur
        }
    } else {
        std::cout << "[ConfigManager] Dosya bulunamadi, yeni olusturuluyor..." << std::endl;
        saveConfig();
    }
}

// --- LOAD CONFIG ---
bool ConfigManager::loadConfig() {
    std::ifstream file(configFilename);
    if (!file.is_open()) return false;

    std::string line;
    std::vector<std::string> lines;

    // Tüm satırları oku
    while (std::getline(file, line)) {
        if(!line.empty()) lines.push_back(line);
    }
    file.close();

    // Satır sayısı kontrolü (Eksik veri varsa yükleme)
    if (lines.size() < 7) return false;

    try {
        // Geçici değişkenlerle dene (Hata olursa eskiler bozulmasın)
        std::string t_name = lines[0];
        std::string t_ip = lines[1];
        int t_port = std::stoi(lines[2]); // Sayısal hata olabilir (stoi)
        
        std::string t_broadIp = lines[3];
        int t_broadPort = std::stoi(lines[4]);
        
        std::string t_serial = lines[5];
        int t_baud = std::stoi(lines[6]);

        // Doğrulama Kontrolleri
        if (!isValidIP(t_ip)) throw std::invalid_argument("Gecersiz IP");
        if (!isValidPort(t_port)) throw std::invalid_argument("Gecersiz Port");
        if (!isValidIP(t_broadIp)) throw std::invalid_argument("Gecersiz Broadcast IP");
        if (!isValidBaudRate(t_baud)) throw std::invalid_argument("Gecersiz BaudRate");

        // Her şey yolundaysa asıl değişkenlere ata
        name = t_name;
        ipAddress = t_ip;
        port = t_port;
        broadcastingAddress = t_broadIp;
        broadcastingPort = static_cast<uint16_t>(t_broadPort);
        serialPortName = t_serial;
        baudRate = t_baud;

        return true;

    } catch (const std::exception& e) {
        std::cerr << "[ConfigManager] Okuma Hatasi: " << e.what() << std::endl;
        return false;
    }
}

// --- SAVE CONFIG ---
bool ConfigManager::saveConfig() const {
    std::ofstream outfile(configFilename);
    if (!outfile.is_open()) return false;

    // Sıralama Önemli: Name -> IP -> Port -> BroadIP -> BroadPort -> Serial -> Baud
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

// --- SETTERS (Validasyonlu) ---
void ConfigManager::setName(const std::string& val) { 
    name = val; saveConfig(); 
}

bool ConfigManager::setIPAddress(const std::string& val) {
    if (!isValidIP(val)) return false;
    ipAddress = val; 
    saveConfig();
    return true;
}

bool ConfigManager::setPort(int val) {
    if (!isValidPort(val)) return false;
    port = val; 
    saveConfig();
    return true;
}

bool ConfigManager::setBroadcastingIPAddress(const std::string& val) {
    if (!isValidIP(val)) return false;
    broadcastingAddress = val; 
    saveConfig();
    return true;
}

bool ConfigManager::setBroadcastingPort(uint16_t val) {
    if (!isValidPort(val)) return false;
    broadcastingPort = val; 
    saveConfig();
    return true;
}

void ConfigManager::setSerialPortName(const std::string& val) { 
    serialPortName = val; saveConfig(); 
}

bool ConfigManager::setBaudRate(int val) {
    if (!isValidBaudRate(val)) return false;
    baudRate = val; 
    saveConfig();
    return true;
}
