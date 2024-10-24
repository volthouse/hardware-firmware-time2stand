#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <chrono>
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include <iostream>

class AppSettings {
public:

    static AppSettings& Instance() {
        static AppSettings instance;
        return instance;
    }

    void Init(const std::string fileName);

    void Save() const;

    void Load();

    void setPortName(const std::string& portName);
    void setStandTotalDuration(const int duration);

    std::string PortName() const;
    int StandTotalDuration() const;

private:
    AppSettings() = default;
    AppSettings(const AppSettings&) = delete;
    AppSettings& operator=(const AppSettings&) = delete;
    ~AppSettings();

    std::string m_filename;
    std::string m_portName;
    int m_standTotalDuration = 0;
};


#endif // APPSETTINGS_H
