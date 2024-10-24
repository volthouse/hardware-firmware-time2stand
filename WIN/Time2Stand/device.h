#ifndef DEVICE2_H
#define DEVICE2_H

#include <iostream>
#include <mutex>
#include <string>
#include <system_error>
#include <vector>
#include <windows.h>

struct DeviceData {
    uint16_t distance;
    uint16_t count;
};

class Device {
public:

    static Device& Instance() {
        static Device instance;
        return instance;
    }

    std::error_code Open(const std::string &portName, uint32_t baudrate);
    void Close();

    std::error_code ReceiveData(DeviceData &data);
    std::error_code ResetData();

    void TestSit();
    void TestStand();

private:
    Device();
    ~Device();

    bool calculateChecksum(const std::vector<uint8_t>& buffer);
    int configure();
    std::string portNameToSystemLocation(const std::string& source);
    std::wstring stringToWString(const std::string& str);
    std::error_code send(const std::string& text);
    std::error_code send(void *buffer, const uint32_t len);
    std::error_code receive(void *buffer, const uint32_t len);
    void purge();

private:
    std::string m_portname;
    uint32_t m_baudrate;
    uint32_t m_read_timeout_ms = 1000;
    uint32_t m_send_timeout_ms = 1000;
    HANDLE m_handle;
    std::mutex m_mutex;

};

#endif // DEVICE2_H
