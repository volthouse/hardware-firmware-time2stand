#include "appsettings.h"

AppSettings::~AppSettings()
{
    Save();
}

void AppSettings::Init(const std::string fileName)
{
    m_filename = fileName;
}

void AppSettings::Save() const {
    nlohmann::json j;
    j["portName"] = m_portName;
    j["standTotalDuration"] = m_standTotalDuration;

    std::ofstream o(m_filename);
    if (o.is_open()) {
        o << j.dump(4);
        o.close();
    } else {
        std::cerr << "Fehler beim Öffnen der Datei zum Speichern." << std::endl;
    }
}

void AppSettings::Load() {
    std::ifstream i(m_filename);
    if (i.is_open()) {
        nlohmann::json j;
        i >> j;
        m_portName = j.value("portName", "COM1");
        m_standTotalDuration = j.value("standTotalDuration", 0);
        i.close();
    } else {
        m_portName = "COM1";
        m_standTotalDuration = 0;
        Save(); // Speichern der Standardwerte in die Datei
    }
}

void AppSettings::setPortName(const std::string& portName) {
    m_portName = portName;
}

void AppSettings::setStandTotalDuration(const int duration)
{
    m_standTotalDuration = duration;
}

std::string AppSettings::PortName() const
{
    return m_portName;
}

int AppSettings::StandTotalDuration() const
{
    return m_standTotalDuration;
}
