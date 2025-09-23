#include "../include/config_manager.h"
#include <fstream>
#include <iostream>

// dosya kontrolu
bool ConfigManager::fileExists() const {
    std::ifstream file(configFilename);
    return file.good();
}

ConfigManager::ConfigManager(const std::string& filename)
    : configFilename(filename), name("HermodPrototypeVehicle"), ipAddress("192.168.1.100"), port(12345), broadcastingAddress("255.255.255.255"), broadcastingPort(12321)
{
    //eger dosya varsa dosyayi oku eger oksa default degerlerle yeni dosya kaydet
    if (!fileExists() || !loadConfig()) {
        if (!saveConfig()) {
            std::cerr << "config kaydedilemdei: " << configFilename << std::endl;
        }
    }
}

bool ConfigManager::loadConfig() {
    std::ifstream file(configFilename);
    if (!file.is_open()) {
        return false;
    }

    //
    std::getline(file, name);
    std::getline(file, ipAddress);

    std::string portText = "";
    std::getline(file, portText);
    port = std::stoi(portText);

    std::getline(file, broadcastingAddress);
    
    std::string broadcastingPortText = "";
    std::getline(file, broadcastingPortText);
    broadcastingPort = std::stoi(broadcastingPortText);

    file.close();
    return true;
}

bool ConfigManager::saveConfig() const {
    std::ofstream outfile(configFilename);
    if (!outfile.is_open()) {
        return false;
    }

    // verileri yaz
    outfile << name << std::endl;
    outfile << ipAddress << std::endl;
    outfile << port << std::endl;

    outfile << broadcastingAddress << std::endl;
    outfile << broadcastingPort << std::endl;

    outfile.close();
    return true;
}

std::string ConfigManager::getName() const {
    return name;
}

std::string ConfigManager::getIPAddress() const {
    return ipAddress;
}

std::string ConfigManager::getBroadcastingIPAddress() const {
    return broadcastingAddress;
}

u_int16_t ConfigManager::getBroadcastingPort() const {
    return broadcastingPort;
}

int ConfigManager::getPort() const {
    return port;
}

void ConfigManager::setName(const std::string& name) {
    this->name = name;
    saveConfig();
}

void ConfigManager::setIPAddress(const std::string& ip) {
    ipAddress = ip;
    saveConfig();
}

void ConfigManager::setBroadcastingIPAddress(const std::string& ip) {
    broadcastingAddress = ip;
    saveConfig();
}

void ConfigManager::setBroadcastingPort(const int port) {
    broadcastingPort = port;
    saveConfig();
}

void ConfigManager::setPort(const int port) {
    this->port = port;
    saveConfig();
}
